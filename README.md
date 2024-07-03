## Summary
The `PowershellExecutor` class is designed to execute PowerShell scripts and handle various types of output logs such as verbose, warning, error, and information. It binds to PowerShell streams to capture and process these logs, sending them to a native logging function via P/Invoke. The class also includes mechanisms to handle activity log thresholds and specific error details, such as web exceptions.

## Build

`mkdir build`
`cd build`
`cmake ..`

## Example Usage
```csharp
IntPtr callbacks = // obtain the callback pointer
PowershellExecutor executor = new PowershellExecutor(callbacks);

PowerShell ps = PowerShell.Create();
DefaultHost host = new DefaultHost();

executor.BindEvents(ps, host);

string script = "Get-Process";
bool isInlinePowershell = true;

try
{
    PowerShellExecutionResult result = executor.ExecutePowerShell(script, isInlinePowershell);
    // Process result
}
catch (NotImplementedException ex)
{
    Console.WriteLine(ex.Message);
}
```

## Code Analysis
### Main functionalities
- Execute PowerShell scripts.
- Capture and process different types of PowerShell output logs.
- Send logs to a native logging function.
- Handle activity log thresholds.
- Bind to PowerShell streams for logging.
___
### Methods
- `SendLog`: Sends log messages to a native logging function.
- `Debug_DataAdded`: Placeholder for handling debug data added events.
- `Progress_DataAdded`: Placeholder for handling progress data added events.
- `Error_DataAdded`: Processes error records and sends detailed error logs.
- `Verbose_DataAdded`: Processes verbose records and sends verbose logs.
- `Warning_DataAdded`: Processes warning records and sends warning logs.
- `Information_DataAdded`: Placeholder for handling information data added events.
- `OnOutputDataReceived`: Handles output data received events and sends information logs.
- `Host_OnInformation`: Handles host information events and sends information logs.
- `HandleInformation`: Sends information logs.
- `BindEvents`: Binds the class methods to PowerShell stream events.
- `ExecutePowerShell`: Placeholder for executing PowerShell scripts.
___
### Fields
- `callbacks`: Pointer to native callback functions.
- `VerboseLinesProcessed`: Counter for processed verbose lines.
- `WarningLinesProcessed`: Counter for processed warning lines.
- `ErrorLinesProcessed`: Counter for processed error lines.
- `InformationLinesProcessed`: Counter for processed information lines.
- `ActivityLogCounter`: Counter for activity logs.
- `ActivityLogThreshold`: Threshold for activity logs.
___
