// PowershellExecutor (C++/CLI)
PowershellExecutor::.ctor
PowershellExecutor::Debug_DataAdded
PowershellExecutor::SendLog
PowershellExecutor::Error_DataAdded
PowershellExecutor::Verbose_DataAdded
PowershellExecutor::Warning_DataAdded
PowershellExecutor::OnOutputDataReceived
PowershellExecutor::DataAddedEventHandler
PowershellExecutor::Information_DataAdded
PowershellExecutor::OnOutputDataReceived
PowershellExecutor::HandleInformation
PowershellExecutor::BindEvents
PowershellExecutor::ExecutePowerShell
StringUtil::ConvertWstringToUtf8String

// PowershellExecutor.h (C++/CLI Header)
#pragma once

#include <string>
#include <msclr/marshal_cppstd.h>
#include <System.Management.Automation.h>
#include "PowerShellExecutionResult.h"

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Collections::Generic;

ref class PowershellExecutor
{
private:
    Callbacks ^ callbacks;
    int activityLogCounter;
    int verboseLinesProcessed;
    int warningLinesProcessed;
    const int activityLogThreshold = 1000;

    void SendLog(String ^ logMessage, LogOutputType logType);
    void Debug_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Progress_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Error_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Verbose_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Warning_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void Information_DataAdded(Object ^ sender, DataAddedEventArgs ^ e);
    void OnOutputDataReceived(Object ^ sender, DataReceivedEventArgs ^ e);
    void Host_OnInformation(String ^ information);
    void HandleInformation(String ^ logOutput);
    void BindEvents(PowerShell ^ _ps, DefaultHost ^ host);

    void LogException(const std::exception &ex, const std::string &context);
    void LogManagedException(Exception ^ ex, const std::string &context);

public:
    PowershellExecutor(Callbacks ^ callbacks);
    PowerShellExecutionResult ^ ExecutePowerShell(String ^ script, Dictionary<String ^, Object ^> ^ parameters);
};
