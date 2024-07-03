#include "LogUtil.h"
#include <msclr/marshal_cppstd.h>
#include <vcclr.h>

using namespace System::IO;
//using namespace System::Environment;

void LogUtil::LogInfo(String ^ shortMessage, String ^ detailDescription)
{
    Log(LogLevel::Information, shortMessage, detailDescription);
}

void LogUtil::LogError(String ^ shortMessage, String ^ detailDescription)
{
    Log(LogLevel::Error, shortMessage, detailDescription);
}

void LogUtil::LogWarning(String ^ shortMessage, String ^ detailDescription)
{
    Log(LogLevel::Warning, shortMessage, detailDescription);
}

void LogUtil::LogDebug(String ^ shortMessage, String ^ detailDescription)
{
    Log(LogLevel::Debug, shortMessage, detailDescription);
}

void LogUtil::LogVerbose(String ^ shortMessage, String ^ detailDescription)
{
    Log(LogLevel::Verbose, shortMessage, detailDescription);
}

void LogUtil::Log(LogLevel level, String ^ shortDescription, String ^ detailDescription)
{
    String ^ levelString = level.ToString();
    String ^ text = String::Format("{0}\t{1}\t", DateTime::Now.ToString("ddd MMM  d H:mm:ss yyyy:"), levelString);

    if (level == LogLevel::Error)
    {
        text += "\t";
    }

    String ^ logMessage = text + shortDescription;

    if (!String::IsNullOrEmpty(detailDescription))
    {
        logMessage += " For more information please see log file";
    }

    Console::WriteLine(logMessage);

    if (!String::IsNullOrEmpty(detailDescription))
    {
        logMessage = text + detailDescription;
        if (level != LogLevel::Error)
        {
            logMessage = text + shortDescription + "\r\n";
            logMessage += text + "Detail description: " + detailDescription + "\r\n";
        }
    }

    logLines->Add(logMessage);

    if (logLines->Count >= maxLogLines)
    {
        WriteLogsToFile();
    }
}

void LogUtil::FlushLogs()
{
    WriteLogsToFile();
}

void LogUtil::WriteLogsToFile()
{
    String ^ agentInstallationDirectory = GetAgentInstallationDirectory();
    try
    {
        if (Directory::Exists(agentInstallationDirectory))
        {
            String ^ logFilePath = Path::Combine(agentInstallationDirectory, logFileName);
            File::AppendAllLines(logFilePath, logLines);
            logLines->Clear();
        }
    }
    catch (Exception ^ ex)
    {
        Console::WriteLine("Error writing to log file: " + ex->Message);
    }
}

String ^ LogUtil::GetAgentInstallationDirectory()
{
    String ^ folderPath = Environment::GetFolderPath(Environment::SpecialFolder::CommonApplicationData);
    return Path::Combine(folderPath, "OrchestratorAgent", "Logs");
}