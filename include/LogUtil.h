#pragma once

#include <string>
#include <vector>

class LogUtil
{
public:
    static void LogInfo(const std::string &shortMessage, const std::string &detailDescription = "");
    static void LogError(const std::string &shortMessage, const std::string &detailDescription = "");
    static void LogError(const EHExceptionRecord &exceptionRecord);
    static void LogWarning(const std::string &shortMessage, const std::string &detailDescription = "");
    static void LogDebug(const std::string &shortMessage, const std::string &detailDescription = "");
    static void LogVerbose(const std::string &shortMessage, const std::string &detailDescription = "");

private:
    static void Log(const std::string &type, const std::string &shortDescription, const std::string &detailDescription = "");
    static std::string GetAgentInstallationDirectory();

    static std::vector<std::string> logLines;
};

struct EHExceptionRecord
{
    std::string Message;
    std::string ToString() const;
};