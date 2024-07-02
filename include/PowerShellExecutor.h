#pragma once

#include <string>
#include <msclr/marshal_cppstd.h>
#include <vcclr.h>
#include <memory>

using namespace System;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Runspaces;
using namespace System::Diagnostics;

// Callback function type
typedef void (*LogCallbackFunction)(const char *, int);

// Callbacks structure
struct Callbacks
{
    LogCallbackFunction LogCallback;
};

public
enum class LogOutputType
{
    Error = 1,
    Verbose = 2,
    Information = 3,
    Warning = 4,
    Debug = 5
};

public
ref class PowerShellExecutor
{
public:
    PowerShellExecutor(IntPtr callbacks);

    /// <summary>
    /// Executes a PowerShell script.
    /// </summary>
    /// <param name="script">The script to execute.</param>
    /// <param name="isInlinePowershell">Whether the script is inline PowerShell.</param>
    /// <returns>A PowerShellExecutionResult containing the execution result.</returns>
    // PowerShellExecutionResult ExecutePowerShell(String ^ script, bool isInlinePowershell);
    [return:MarshalAs(UnmanagedType::U1)] PowerShellExecutionResult *ExecutePowerShell(PowerShellExecutionResult *result, String ^ script, [MarshalAs(UnmanagedType::U1)] bool isInlinePowershell);

private:
    void SendLog(String ^ logOutput, LogOutputType logType);
    void BindEvents(PowerShell ^ ps, DefaultHost ^ host);
    String ^ DeserializeScriptVariables(String ^ script);
    void Debug_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Progress_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Error_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Verbose_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Warning_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Information_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void OnOutputDataReceived(Object ^ sender, DataReceivedEventArgs ^ e);
    void Host_OnInformation(String ^ information);
    void HandleInformation(String ^ logOutput);
    void LogManagedException(Exception ^ ex, String ^ context);

    IntPtr callbacks;
    int verboseLinesProcessed;
    int warningLinesProcessed;
    int errorLinesProcessed;
    int informationLinesProcessed;
    int activityLogCounter;
    int activityLogThreshold;
};

public
ref class PowerShellExecutionResult
{
public:
    property bool Success;
    property String ^ Output;
    property String ^ ErrorMessage;
};