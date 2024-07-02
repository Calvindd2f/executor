#pragma once

using namespace System;
using namespace System::Collections::Generic;

#include <string>
#include <vector>

public
enum class LogLevel
{
    Debug,
    Verbose,
    Information,
    Warning,
    Error
};

public
ref class LogUtil
{
public:
    static void LogInfo(String ^ shortMessage, String ^ detailDescription);
    static void LogError(String ^ shortMessage, String ^ detailDescription);
    static void LogWarning(String ^ shortMessage, String ^ detailDescription);
    static void LogDebug(String ^ shortMessage, String ^ detailDescription);
    static void LogVerbose(String ^ shortMessage, String ^ detailDescription);
    static void FlushLogs();

private:
    static void Log(LogLevel level, String ^ shortDescription, String ^ detailDescription);
    static String ^ GetAgentInstallationDirectory();
    static void WriteLogsToFile();

    static List<String ^> ^ logLines = gcnew List<String ^>();
    static String ^ logFileName = "LogFile_Installer.txt";
    static int maxLogLines = 1000; // Adjust as needed
};