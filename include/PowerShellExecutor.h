#pragma once

#include <string>
#include <msclr/marshal_cppstd.h>
#include <vcclr.h>

using namespace System;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Runspaces;
using namespace System::Diagnostics;

public
ref class PowerShellExecutor
{
public:
    PowerShellExecutor(IntPtr callbacks);
    void SendLog(String ^ logOutput, LogOutputType logType);

    void Debug_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Progress_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Error_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Verbose_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Warning_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Information_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void OnOutputDataReceived(Object ^ sender, DataReceivedEventArgs ^ e);
    void Host_OnInformation(String ^ information);
    void HandleInformation(String ^ logOutput);
    void BindEvents(PowerShell ^ ps, DefaultHost ^ host);

    [return:MarshalAs(UnmanagedType::U1)] PowerShellExecutionResult *ExecutePowerShell(PowerShellExecutionResult *result, String ^ script, [MarshalAs(UnmanagedType::U1)] bool isInlinePowershell);

private:
    IntPtr callbacks;
    int verboseLinesProcessed;
    int warningLinesProcessed;
    int errorLinesProcessed;
    int informationLinesProcessed;
    int activityLogCounter;
    int activityLogThreshold;
};

// Assuming this is the structure of your PowerShellExecutionResult
struct PowerShellExecutionResult
{
    bool Success;
    char *Output;
};