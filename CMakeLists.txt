# CMakeLists.txt for OrchestratorAgent with C++/CLI and C# components

cmake_minimum_required(VERSION 3.15)
project(OrchestratorAgent LANGUAGES CXX CSharp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable C++/CLI
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_CLANG_TIDY "")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Download and configure NuGet
if(NOT EXISTS "${CMAKE_BINARY_DIR}/nuget.exe")
    file(DOWNLOAD
        "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
        "${CMAKE_BINARY_DIR}/nuget.exe"
    )
endif()

# Function to add NuGet packages
function(use_nuget_package TARGET PACKAGE VERSION)
    execute_process(
        COMMAND ${CMAKE_BINARY_DIR}/nuget.exe install ${PACKAGE} -Version ${VERSION} -OutputDirectory ${CMAKE_BINARY_DIR}/packages
    )
    file(GLOB PACKAGE_DIR "${CMAKE_BINARY_DIR}/packages/${PACKAGE}*")
    file(GLOB_RECURSE ASSEMBLY_PATHS "${PACKAGE_DIR}/*.dll")

    foreach(ASSEMBLY_PATH ${ASSEMBLY_PATHS})
        get_filename_component(ASSEMBLY_NAME ${ASSEMBLY_PATH} NAME_WE)
        set_property(TARGET ${TARGET} APPEND PROPERTY
            VS_DOTNET_REFERENCE_${ASSEMBLY_NAME} ${ASSEMBLY_PATH}
        )
    endforeach()
endfunction()

# C++/CLI Library
add_library(NativeExecutor SHARED
    src/EHExceptionRecord.cpp
    src/LogUtil.cpp
    src/PowerShellExecutionResult.cpp
    src/PowerShellExecutor.cpp
    src/ProcessUtil.cpp
)

target_include_directories(NativeExecutor PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(NativeExecutor PRIVATE /clr)

# Set properties for C++/CLI library
set_target_properties(NativeExecutor PROPERTIES
    COMMON_LANGUAGE_RUNTIME "netcore" # Use "netcore" for .NET Core or "" for .NET Framework
    VS_GLOBAL_CLRSupport "true"
    VS_DOTNET_REFERENCES "System;System.Management.Automation"
)

# Add NuGet packages for C++/CLI
use_nuget_package(NativeExecutor "Microsoft.PowerShell.5.1.ReferenceAssemblies" "1.0.0")

# C# Library
set(CS_LIBRARY_SOURCES
    managed/DefaultHost.cs
    managed/DefaultHostRawUserInterface.cs
    managed/DefaultHostUserInterface.cs
    managed/LogOutputType.cs
    managed/PowerShellExecutionResult.cs
    managed/PowershellExecutor.cs
)

add_library(ManagedExecutor SHARED ${CS_LIBRARY_SOURCES})

set_target_properties(ManagedExecutor PROPERTIES
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
    DOTNET_SDK "Microsoft.NET.Sdk"
    FOLDER "CSharp"
)

# Add dependencies
add_dependencies(ManagedExecutor NativeExecutor)

# Reference NativeExecutor in ManagedExecutor
set_property(TARGET ManagedExecutor APPEND PROPERTY
    VS_DOTNET_REFERENCES "${CMAKE_BINARY_DIR}/lib/NativeExecutor.dll"
)

# Add NuGet packages for C#
use_nuget_package(ManagedExecutor "JetBrains.Annotations" "2021.2.0")
use_nuget_package(ManagedExecutor "Json.More.Net" "1.9.3")
use_nuget_package(ManagedExecutor "JsonPointer.Net" "3.0.3")
use_nuget_package(ManagedExecutor "JsonSchema.Net" "5.2.7")
use_nuget_package(ManagedExecutor "Markdig.Signed" "0.33.0")
use_nuget_package(ManagedExecutor "Microsoft.ApplicationInsights" "2.21.0")
use_nuget_package(ManagedExecutor "Microsoft.Bcl.AsyncInterfaces" "8.0.0")
use_nuget_package(ManagedExecutor "Microsoft.CodeAnalysis.Analyzers" "3.3.4")
use_nuget_package(ManagedExecutor "Microsoft.CodeAnalysis.Common" "4.8.0")
use_nuget_package(ManagedExecutor "Microsoft.CodeAnalysis.CSharp" "4.8.0")
use_nuget_package(ManagedExecutor "Microsoft.Extensions.ObjectPool" "8.0.6")
use_nuget_package(ManagedExecutor "Microsoft.Management.Infrastructure" "3.0.0")
use_nuget_package(ManagedExecutor "Microsoft.Management.Infrastructure.CimCmdlets" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.Management.Infrastructure.Runtime.Unix" "3.0.0")
use_nuget_package(ManagedExecutor "Microsoft.Management.Infrastructure.Runtime.Win" "3.0.0")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.SDK" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.Commands.Diagnostics" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.Commands.Management" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.Commands.Utility" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.ConsoleHost" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.CoreCLR.Eventing" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.MarkdownRender" "7.2.1")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.Native" "7.4.0")
use_nuget_package(ManagedExecutor "Microsoft.PowerShell.Security" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.Security.Extensions" "1.2.0")
use_nuget_package(ManagedExecutor "Microsoft.Win32.Registry" "5.0.0")
use_nuget_package(ManagedExecutor "Microsoft.Win32.Registry.AccessControl" "8.0.0")
use_nuget_package(ManagedExecutor "Microsoft.Win32.SystemEvents" "8.0.0")
use_nuget_package(ManagedExecutor "Microsoft.Windows.Compatibility" "8.0.6")
use_nuget_package(ManagedExecutor "Microsoft.WSMan.Management" "7.4.3")
use_nuget_package(ManagedExecutor "Microsoft.WSMan.Runtime" "7.4.3")
use_nuget_package(ManagedExecutor "Newtonsoft.Json" "13.0.3")
use_nuget_package(ManagedExecutor "PowerShellStandard.Library" "5.1.1")
use_nuget_package(ManagedExecutor "System.Management.Automation" "7.4.3")

# Install rules
install(TARGETS NativeExecutor ManagedExecutor
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

install(FILES "${CMAKE_SOURCE_DIR}/libs/PowerShellRuntimeExtensions.dll"
    DESTINATION bin
)

# Copy header files to include directory in build
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)