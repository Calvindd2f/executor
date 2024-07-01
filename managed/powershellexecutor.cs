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
        if (ActivityLogCounter > ActivityLogThreshold)
        {
            throw new Exception("Activity Log threshold exceeded.");
        }
        string utf8LogOutput = StringUtil.ConvertWstringToUtf8String(logOutput);
        // Call the native log function using P/Invoke
        NativeMethods.SendLog(callbacks, utf8LogOutput, (int)logType);
        ActivityLogCounter++;
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
        ps.Streams.Debug.DataAdded += Debug_DataAdded;
        ps.Streams.Error.DataAdded += Error_DataAdded;
        ps.Streams.Progress.DataAdded += Progress_DataAdded;
        ps.Streams.Verbose.DataAdded += Verbose_DataAdded;
        ps.Streams.Warning.DataAdded += Warning_DataAdded;

        PropertyInfo informationProperty = ps.GetType().GetProperty("Information");
        if (informationProperty != null)
        {
            object informationObject = informationProperty.GetValue(ps.Streams);
            EventInfo dataAddedEvent = informationObject.GetType().GetEvent("DataAdded");
            MethodInfo informationMethod = GetType().GetMethod("Information_DataAdded", BindingFlags.Instance | BindingFlags.NonPublic);
            Delegate handler = Delegate.CreateDelegate(dataAddedEvent.EventHandlerType, this, informationMethod);
            dataAddedEvent.AddEventHandler(informationObject, handler);
        }
        else
        {
            host.OnInformation += Host_OnInformation;
        }
    }

    public PowerShellExecutionResult ExecutePowerShell(string script, bool isInlinePowershell)
    {
        // Implementation of ExecutePowerShell method
        // This would involve creating a PowerShell instance, setting up the runspace,
        // executing the script, and handling the results.
        // The actual implementation would be quite extensive and would need to mirror
        // the functionality in the C++/CLR version.

        throw new NotImplementedException("ExecutePowerShell needs to be implemented");
    }
}
