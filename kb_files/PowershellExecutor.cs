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
	public unsafe PowershellExecutor(Callbacks* callbacks)
	{
		this.activityLogCounter = 0;
		this.callbacks = callbacks;
	}

	public unsafe void SendLog(string logOutput, LogOutputType logtype)
	{
		if (this.activityLogCounter > this.activityLogThreshold)
		{
			Callbacks* ptr = this.callbacks;
			throw new Exception("Activity Log threshold exceeded.");
		}
		StringUtil.ConvertWstringToUtf8String;
		this.activityLogCounter++;
	}

	public void Debug_DataAdded(object sender, DataAddedEventArgs e)
	{
	}

	public void Progress_DataAdded(object sender, DataAddedEventArgs e)
	{
	}

	public unsafe void Error_DataAdded(object sender, DataAddedEventArgs e)
	{
	}

	public void Verbose_DataAdded(object sender, DataAddedEventArgs e) 
	{
	}

	public void Warning_DataAdded(object sender, DataAddedEventArgs e)
	{
	}

	public void Information_DataAdded(object sender, DataAddedEventArgs e)
	{
	}

	public void OnOutputDataReceived(object sender, DataReceivedEventArgs e)
	{
		string data = e.Data;
		if (data.Length > 0)
		{
			this.SendLog(data, (LogOutputType)3);
		}
		GC.KeepAlive(this);
	}

	public void Host_OnInformation(string information)
	{
		string text = information.Trim();
		if (text.Length > 0)
		{
			this.SendLog(text, (LogOutputType)3);
		}
		GC.KeepAlive(this);
	}

	public void HandleInformation(string logOutput)
	{
		if (logOutput.Length > 0)
		{
			this.SendLog(logOutput, (LogOutputType)3);
		}
	}

	public void BindEvents(PowerShell _ps, DefaultHost host)
	{
		PSDataCollection<DebugRecord> debug = _ps.Streams.Debug;
		EventHandler<DataAddedEventArgs> eventHandler = new EventHandler<DataAddedEventArgs>(this.Debug_DataAdded);
		debug.DataAdded += eventHandler;
		PSDataCollection<ErrorRecord> error = _ps.Streams.Error;
		EventHandler<DataAddedEventArgs> eventHandler2 = new EventHandler<DataAddedEventArgs>(this.Error_DataAdded);
		error.DataAdded += eventHandler2;
		PSDataCollection<ProgressRecord> progress = _ps.Streams.Progress;
		EventHandler<DataAddedEventArgs> eventHandler3 = new EventHandler<DataAddedEventArgs>(this.Progress_DataAdded);
		progress.DataAdded += eventHandler3;
		Type type = _ps.GetType();
		string text = "Information";
		PropertyInfo property = type.GetProperty(text);
		if (property != null)
		{
			PropertyInfo[] array = null;
			object value = property.GetValue(_ps.Streams, array);
			Type type2 = value.GetType();
			string text2 = "DataAdded";
			EventInfo @event = type2.GetEvent(text2);
			Type type3 = base.GetType();
			string text3 = "Information_DataAdded";
			MethodInfo method = type3.GetMethod(text3, BindingFlags.Instance | BindingFlags.NonPublic);
			Delegate @delegate = Delegate.CreateDelegate(@event.EventHandlerType, this, method);
			@event.AddEventHandler(value, @delegate);
		}
		else
		{
			host.OnInformation += this.Host_OnInformation;
		}
		PSDataCollection<VerboseRecord> verbose = _ps.Streams.Verbose;
		EventHandler<DataAddedEventArgs> eventHandler4 = new EventHandler<DataAddedEventArgs>(this.Verbose_DataAdded);
		verbose.DataAdded += eventHandler4;
		PSDataCollection<WarningRecord> warning = _ps.Streams.Warning;
		EventHandler<DataAddedEventArgs> eventHandler5 = new EventHandler<DataAddedEventArgs>(this.Warning_DataAdded);
		warning.DataAdded += eventHandler5;
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

	private unsafe Callbacks* callbacks = null;

	public int verboseLinesProcessed = 0;

	public int warningLinesProcessed = 0;

	public int errorLinesProcessed = 0;

	public int informationLinesProcessed = 0;

	public int activityLogCounter = 0;

	public int activityLogThreshold = 1000;
}
