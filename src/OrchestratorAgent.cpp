// src/OrchestratorAgent.cpp
#include "OrchestratorAgent.h"
#include "PowerShellExecutor.h"
#include <iostream>
#include <string>
#include <vector>

OrchestratorAgent::OrchestratorAgent()
{
    m_powerShellExecutor = std::make_unique<PowerShellExecutor>();
}

void OrchestratorAgent::Start()
{
    std::cout << "Orchestrator Agent started." << std::endl;
    // TODO: Implement the main loop for handling incoming requests
}

std::string OrchestratorAgent::ExecutePowerShellScript(const std::string &script, const std::vector<std::pair<std::string, std::string>> &parameters)
{
    return m_powerShellExecutor->ExecuteScript(script, parameters);
}