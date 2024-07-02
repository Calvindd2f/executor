#include "LogUtil.h"
#include <msclr/marshal_cppstd.h>
#include <vcclr.h>

using namespace System::IO;
using namespace System::Environment;

void LogUtil::LogInfo(String ^ shortMessage, String ^ detailDescription)
{
    Log("Informational", shortMessage, detailDescription);
}

void LogUtil::LogError(String ^ shortMessage, String ^ detailDescription)
{
    Log("Error", shortMessage, detailDescription);
}

void LogUtil::LogWarning(String ^ shortMessage, String ^ detailDescription)
{
    Log("Warning", shortMessage, detailDescription);
}

void LogUtil::Log(String ^ type, String ^ shortDescription, String ^ detailDescription)
{
    String ^ text = DateTime::Now.ToString("ddd MMM  d H:mm:ss yyyy:") + "\t" + type + "\t";
    if (type == "Error")
    {
        text += "\t";
    }
    String ^ text2 = text + shortDescription;
    if (!String::IsNullOrEmpty(detailDescription))
    {
        text2 += " For more information please see log file";
    }
    Console::WriteLine(text2);
    if (!String::IsNullOrEmpty(detailDescription))
    {
        text2 = text + detailDescription;
        if (type != "Error")
        {
            text2 = text + shortDescription + "\r\n";
            text2 = text2 + "Detail description: " + detailDescription + "\r\n";
        }
    }
    logLines->Add(text2);
    String ^ agentInstallationDirectory = GetAgentInstallationDirectory();
    try
    {
        if (Directory::Exists(agentInstallationDirectory))
        {
            File::AppendAllLines(Path::Combine(agentInstallationDirectory, "LogFile_Installer.txt"), logLines);
            logLines->Clear();
        }
    }
    catch (DirectoryNotFoundException ^)
    {
    }
    catch (UnauthorizedAccessException ^)
    {
    }
}

String ^ LogUtil::GetAgentInstallationDirectory()
{
    String ^ folderPath = Environment::GetFolderPath(Environment::SpecialFolder::ProgramFiles);
    String ^ text = Path::Combine(folderPath, "OrchestratorAgent");
    return Path::Combine(folderPath, text);
}