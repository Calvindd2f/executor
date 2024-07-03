using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Net;
using System.Reflection;
using System.Runtime.InteropServices;

internal class PowershellExecutor
{
    private readonly IntPtr callbacks;
    public int VerboseLinesProcessed { get; set; } = 0;
    public int WarningLinesProcessed { get; set; } = 0;
    public int ErrorLinesProcessed { get; set; } = 0;
    public int InformationLinesProcessed { get; set; } = 0;
    public int ActivityLogCounter { get; set; } = 0;
    public int ActivityLogThreshold { get; set; } = 1000;

    public PowershellExecutor(IntPtr callbacks)
    {
        this.callbacks = callbacks;
    }

    public void SendLog(string logOutput, LogOutputType logType)
    {
        if (string.IsNullOrEmpty(logOutput))
        {
            throw new ArgumentException("Log output cannot be null or empty.", nameof(logOutput));
        }

        try
        {
            if (ActivityLogCounter > ActivityLogThreshold)
            {
                throw new Exception("Activity Log threshold exceeded.");
            }

            string utf8LogOutput = StringUtil.ConvertWstringToUtf8String(logOutput);

            if (callbacks == IntPtr.Zero)
            {
                throw new InvalidOperationException("Callbacks pointer is not initialized.");
            }

            NativeMethods.SendLog(callbacks, utf8LogOutput, (int)logType);
            ActivityLogCounter++;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error occurred while sending log: {ex}");
            throw;
        }
    }

    public void Debug_DataAdded(object sender, DataAddedEventArgs e)
    {
        // Implement if needed
    }

    public void Progress_DataAdded(object sender, DataAddedEventArgs e)
    {
        // Implement if needed
    }

    public void Error_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<ErrorRecord> errorRecords)
        {
            foreach (ErrorRecord errorRecord in errorRecords)
            {
                string errorMessage = errorRecord.ToString();
                SendLog(errorMessage, LogOutputType.Error);

                if (errorRecord.Exception != null)
                {
                    if (errorRecord.Exception.InnerException is WebException webEx)
                    {
                        try
                        {
                            using (StreamReader reader = new StreamReader(webEx.Response.GetResponseStream()))
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

                    string stackTrace = errorRecord.ScriptStackTrace;
                    if (!string.IsNullOrEmpty(stackTrace))
                    {
                        SendLog("Script Stack Trace: " + stackTrace, LogOutputType.Error);
                    }
                }
            }
        }
    }

    public void Verbose_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<VerboseRecord> verboseRecords)
        {
            string text = "";
            for (int i = VerboseLinesProcessed; i < verboseRecords.Count; i++)
            {
                text += verboseRecords[i].Message;
                VerboseLinesProcessed++;
            }
            SendLog(text, LogOutputType.Verbose);
        }
    }

    public void Warning_DataAdded(object sender, DataAddedEventArgs e)
    {
        if (sender is PSDataCollection<WarningRecord> warningRecords)
        {
            string text = "";
            for (int i = WarningLinesProcessed; i < warningRecords.Count; i++)
            {
                text += warningRecords[i].Message;
                WarningLinesProcessed++;
            }
            SendLog(text, LogOutputType.Warning);
        }
    }

    public void Information_DataAdded(object sender, DataAddedEventArgs e)
    {
        // Implement if needed
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

    public void BindEvents(PowerShell ps, DefaultHost host)
    {
        ps.Streams.Debug.DataAdded += new EventHandler<DataAddedEventArgs>(this.Debug_DataAdded);
        ps.Streams.Error.DataAdded += new EventHandler<DataAddedEventArgs>(this.Error_DataAdded);
        ps.Streams.Progress.DataAdded += new EventHandler<DataAddedEventArgs>(this.Progress_DataAdded);
        ps.Streams.Verbose.DataAdded += new EventHandler<DataAddedEventArgs>(this.Verbose_DataAdded);
        ps.Streams.Warning.DataAdded += new EventHandler<DataAddedEventArgs>(this.Warning_DataAdded);

        PropertyInfo property = ps.GetType().GetProperty("Information");
        if (property != null)
        {
            object value = property.GetValue(ps.Streams, null);
            EventInfo eventInfo = value.GetType().GetEvent("DataAdded");
            MethodInfo methodInfo = this.GetType().GetMethod("Information_DataAdded", BindingFlags.Instance | BindingFlags.NonPublic);
            Delegate delegateInfo = Delegate.CreateDelegate(eventInfo.EventHandlerType, this, methodInfo);
            eventInfo.AddEventHandler(value, delegateInfo);
        }
        else
        {
            host.OnInformation += this.Host_OnInformation;
        }

        GC.KeepAlive(this);
    }

    public unsafe PowerShellExecutionResult* ExecutePowerShell(PowerShellExecutionResult* result, string script, bool isInlinePowershell)
    {
        PowerShellExecutionResult powerShellExecutionResult = new PowerShellExecutionResult();

        using (Runspace runspace = RunspaceFactory.CreateRunspace(new DefaultHost(CultureInfo.CurrentCulture, CultureInfo.CurrentUICulture)))
        {
            runspace.Open();
            using (PowerShell powerShell = PowerShell.Create())
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

                this.BindEvents(powerShell, (DefaultHost)runspace.InitialSessionState.Host);

                if (isInlinePowershell)
                {
                    powerShell.AddScript(script, true);
                }
                else
                {
                    powerShell.AddCommand(script);
                }

                PSInvocationSettings psInvocationSettings = new PSInvocationSettings()
                {
                    Host = (DefaultHost)runspace.InitialSessionState.Host
                };

                using (PSDataCollection<object> outputCollection = new PSDataCollection<object>())
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

                    PSObject psObject = outputCollection[0];
                    string resultString = psObject.ToString();

                    string utf8Result = Encoding.UTF8.GetString(Encoding.Default.GetBytes(resultString));
                    *result = new PowerShellExecutionResult() { Result = utf8Result, Status = 0 };
                }
            }
        }

        return result;
    }

    public int verboseLinesProcessed = 0;
    public int warningLinesProcessed = 0;
    public int errorLinesProcessed = 0;
    public int informationLinesProcessed = 0;
}