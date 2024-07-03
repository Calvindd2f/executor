#pragma once

#include <msclr/marshal_cppstd.h>
#include <vcclr.h>
#include <string>

using namespace System;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Runspaces;

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
ref class PowerShellExecutionResult
{
public:
    property bool Success;
    property String ^ Output;
    property String ^ ErrorMessage;
};

public
ref class PowerShellExecutor
{
public:
    PowerShellExecutor(IntPtr callbacks);
    PowerShellExecutionResult ^ ExecutePowerShell(String ^ script, bool isInlinePowershell);

private:
    IntPtr callbacksPtr;
    int activityLogCounter;
    int verboseLinesProcessed;
    int warningLinesProcessed;
    int errorLinesProcessed;
    int informationLinesProcessed;
    static const int ActivityLogThreshold = 1000;

    void SendLog(String ^ logOutput, LogOutputType logType);
    void BindEvents(PowerShell ^ ps, PSHost ^ host);
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
};