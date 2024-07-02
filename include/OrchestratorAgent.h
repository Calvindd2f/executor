// include/OrchestratorAgent.h
#pragma once

#include <memory>
#include <string>
#include <vector>

class PowerShellExecutor;

class OrchestratorAgent
{
public:
    OrchestratorAgent();
    void Start();
    std::string ExecutePowerShellScript(const std::string &script, const std::vector<std::pair<std::string, std::string>> &parameters);

private:
    std::unique_ptr<PowerShellExecutor> m_powerShellExecutor;
};