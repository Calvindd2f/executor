# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(OrchestratorAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(OrchestratorAgent
    src/main.cpp
    src/OrchestratorAgent.cpp
    src/PowerShellExecutor.cpp
    src/PowerShellExecutorWrapper.cpp
)

target_link_libraries(OrchestratorAgent
    System.Management.Automation
)