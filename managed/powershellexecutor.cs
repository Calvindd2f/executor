using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Net;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

public class PowerShellExecutor
{
    private readonly IntPtr callbacksPtr;
    private int activityLogCounter;
    private const int ActivityLogThreshold = 1000;

    public int VerboseLinesProcessed { get; private set; }
    public int WarningLinesProcessed { get; private set; }
    public int ErrorLinesProcessed { get; private set; }
    public int InformationLinesProcessed { get; private set; }

    [StructLayout(LayoutKind.Sequential)]
    private struct Callbacks
    {
        public IntPtr LogCallback;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void LogCallbackDelegate(string message, int logType);

    [DllImport("NativeLibrary.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void StringUtil_ConvertWstringToUtf8String(
        [MarshalAs(UnmanagedType.LPWStr)] string input,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder output,
        int outputSize);

    public PowerShellExecutor(IntPtr callbacks)
    {
        this.callbacksPtr = callbacks;
        this.activityLogCounter = 0;
        this.VerboseLinesProcessed = 0;
        this.WarningLinesProcessed = 0;
        this.ErrorLinesProcessed = 0;
        this.InformationLinesProcessed = 0;
    }

    private void SendLog(string logOutput, LogOutputType logType)
    {
        if (this.activityLogCounter > ActivityLogThreshold)
        {
            throw new Exception("Activity Log threshold exceeded.");
        }

        var callbacks = Marshal.PtrToStructure<Callbacks>(this.callbacksPtr);
        var logCallback = Marshal.GetDelegateForFunctionPointer<LogCallbackDelegate>(callbacks.LogCallback);

        StringBuilder utf8LogOutput = new StringBuilder(logOutput.Length * 2);
        StringUtil_ConvertWstringToUtf8String(logOutput, utf8LogOutput, utf8LogOutput.Capacity);

        logCallback(utf8LogOutput.ToString(), (int)logType);
        this.activityLogCounter++;
    }

    public void Debug_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<DebugRecord> debugRecords)
        {
            string text = string.Join(Environment.NewLine, debugRecords.ReadAll());
            SendLog(text, LogOutputType.Debug);
        }
    }

    public void Progress_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<ProgressRecord> progressRecords)
        {
            foreach (var record in progressRecords.ReadAll())
            {
                string message = $"Activity: {record.Activity}, Status: {record.StatusDescription}, Progress: {record.PercentComplete}%";
                SendLog(message, LogOutputType.Information);
            }
        }
    }

    public void Error_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<ErrorRecord> errorRecords)
        {
            foreach (var errorRecord in errorRecords.ReadAll())
            {
                SendLog(errorRecord.ToString(), LogOutputType.Error);

                if (errorRecord.Exception != null)
                {
                    SendLog(errorRecord.Exception.ToString(), LogOutputType.Error);

                    if (errorRecord.Exception.InnerException is WebException webEx)
                    {
                        try
                        {
                            using (var reader = new StreamReader(webEx.Response.GetResponseStream()))
                            {
                                string responseText = reader.ReadToEnd();
                                SendLog("Web Exception Details: " + responseText, LogOutputType.Error);
                            }
                        }
                        catch (Exception ex)
                        {
                            SendLog("Error reading web exception details: " + ex.ToString(), LogOutputType.Error);
                        }
                    }
                }
            }
        }
    }

    public void Verbose_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<VerboseRecord> verboseRecords)
        {
            string text = string.Join(Environment.NewLine, verboseRecords.ReadAll().Select(r => r.Message));
            SendLog(text, LogOutputType.Verbose);
            VerboseLinesProcessed += verboseRecords.Count;
        }
    }

    public void Warning_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<WarningRecord> warningRecords)
        {
            string text = string.Join(Environment.NewLine, warningRecords.ReadAll().Select(r => r.Message));
            SendLog(text, LogOutputType.Warning);
            WarningLinesProcessed += warningRecords.Count;
        }
    }

    public void Information_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<InformationRecord> informationRecords)
        {
            string text = string.Join(Environment.NewLine, informationRecords.ReadAll().Select(r => r.MessageData.ToString()));
            SendLog(text, LogOutputType.Information);
            InformationLinesProcessed += informationRecords.Count;
        }
    }

    public void OnOutputDataReceived(object sender, DataReceivedEventArgs e)
    {
        if (!string.IsNullOrEmpty(e.Data))
        {
            SendLog(e.Data, LogOutputType.Information);
        }
    }

    public void Host_OnInformation(string information)
    {
        string text = information.Trim();
        if (!string.IsNullOrEmpty(text))
        {
            SendLog(text, LogOutputType.Information);
        }
    }

    public void HandleInformation(string logOutput)
    {
        if (!string.IsNullOrEmpty(logOutput))
        {
            SendLog(logOutput, LogOutputType.Information);
        }
    }

    public void BindEvents(PowerShell ps, PSHost host)
    {
        ps.Streams.Debug.DataAdded += Debug_DataAdded;
        ps.Streams.Error.DataAdded += Error_DataAdded;
        ps.Streams.Progress.DataAdded += Progress_DataAdded;
        ps.Streams.Verbose.DataAdded += Verbose_DataAdded;
        ps.Streams.Warning.DataAdded += Warning_DataAdded;

        var informationProperty = ps.GetType().GetProperty("Information");
        if (informationProperty != null)
        {
            var informationObject = informationProperty.GetValue(ps.Streams);
            var dataAddedEvent = informationObject.GetType().GetEvent("DataAdded");
            var informationMethod = GetType().GetMethod("Information_DataAdded", BindingFlags.Instance | BindingFlags.NonPublic);
            var handler = Delegate.CreateDelegate(dataAddedEvent.EventHandlerType, this, informationMethod);
            dataAddedEvent.AddEventHandler(informationObject, handler);
        }
        else if (host is DefaultHost defaultHost)
        {
            defaultHost.OnInformation += Host_OnInformation;
        }
    }

    public PowerShellExecutionResult ExecutePowerShell(string script, bool isInlinePowershell)
    {
        var result = new PowerShellExecutionResult();

        try
        {
            using (var runspace = RunspaceFactory.CreateRunspace(new DefaultHost(CultureInfo.CurrentCulture, CultureInfo.CurrentUICulture)))
            {
                runspace.Open();
                using (var powerShell = PowerShell.Create())
                {
                    powerShell.Runspace = runspace;

                    // Load the DLL and define custom commands
                    string initScript = @"
                    Add-Type -Path 'PowerShellRuntimeExtensions20.dll';
                    function ConvertFrom-Json20([object] $inputObject) {
                        $err = $null;
                        return [PowerShellRuntimeExtensions.JsonObject]::ConvertFromJson($inputObject, $false, 4, [ref]$err);
                    }
                    function ConvertTo-Json20([object] $inputObject, $depth = 5) {
                        $ctx = New-Object PowerShellRuntimeExtensions.ConvertToJsonContext $depth, $false, $false, 'Default';
                        return [PowerShellRuntimeExtensions.JsonObject]::ConvertToJson($inputObject, [ref]$ctx);
                    }
                    if ($null -eq (Get-Command 'ConvertTo-Json' -ErrorAction SilentlyContinue)) { 
                        New-Alias -Name 'ConvertTo-JSON' -Value 'ConvertTo-Json20' -Scope Global -Force; 
                    }
                    if ($null -eq (Get-Command 'ConvertFrom-Json' -ErrorAction SilentlyContinue)) { 
                        New-Alias -Name 'ConvertFrom-JSON' -Value 'ConvertFrom-Json20' -Scope Global -Force; 
                    }";

                    powerShell.AddScript(initScript).Invoke();

                    this.BindEvents(powerShell, runspace.SessionStateProxy.Host);

                    if (isInlinePowershell)
                    {
                        powerShell.AddScript(script, true);
                    }
                    else
                    {
                        powerShell.AddCommand(script);
                    }

                    var psInvocationSettings = new PSInvocationSettings
                    {
                        Host = runspace.SessionStateProxy.Host
                    };

                    using (var outputCollection = new PSDataCollection<object>())
                    {
                        powerShell.Invoke(null, outputCollection, psInvocationSettings);

                        if (outputCollection.Count == 0)
                        {
                            throw new Exception("Activity did not return a result and/or failed while executing");
                        }
                        if (outputCollection.Count > 1)
                        {
                            throw new Exception("Activity returned more than one result. See below for details");
                        }

                        var psObject = outputCollection[0];
                        result.Output = psObject.ToString();
                        result.Success = true;
                    }
                }
            }
        }
        catch (Exception ex)
        {
            result.Success = false;
            result.ErrorMessage = ex.ToString();
        }

        return result;
    }
}

public class PowerShellExecutionResult
{
    public bool Success { get; set; }
    public string Output { get; set; }
    public string ErrorMessage { get; set; }
}

public enum LogOutputType
{
    Error = 1,
    Verbose = 2,
    Information = 3,
    Warning = 4,
    Debug = 5
}