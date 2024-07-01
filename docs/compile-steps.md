# Steps to Compile OrchestratorAgent.exe Windows Service

## 1. Compile the Native C++ DLL

1. Open your C++ project for the native DLL in Visual Studio.
2. Ensure all necessary headers and implementation files are included:
   - NativeLib.h
   - NativeLib.cpp
   - Any other supporting C++ files
3. Set the project configuration:
   - Set to Release mode
   - Set platform to x64 (assuming 64-bit target)
   - In project properties:
     - C/C++ > Preprocessor: Add NATIVELIB_EXPORTS to Preprocessor Definitions
     - Linker > Advanced: Set "Export All Symbols" to Yes
4. Build the project to generate NativeLib.dll

## 2. Compile the C# PowerShell Executor

1. Open your C# project for the PowerShell executor in Visual Studio.
2. Ensure all necessary files are included:
   - PowershellExecutor.cs
   - PowerShellExecutionResult.cs
   - LogOutputType.cs
   - NativeMethods.cs
   - StringUtil.cs
3. Add a reference to the compiled `NativeLib.dll`
4. Set the project configuration:
   - Set to Release mode
   - Set platform to x64 (assuming 64-bit target)
5. Build the project to generate the PowerShell executor assembly

## 3. Compile the Main Windows Service Application

1. Open your main C# Windows Service project in Visual Studio.
2. Ensure all necessary files and references are included:
   - Your main service implementation file
   - Reference to System.ServiceProcess
   - Reference to the PowerShell executor assembly
   - Reference to NativeLib.dll
3. Set the project configuration:
   - Set to Release mode
   - Set platform to x64 (assuming 64-bit target)
   - Set the output type to Windows Application
4. Implement the Windows service logic:
   - Create a class that inherits from ServiceBase
   - Implement OnStart and OnStop methods
   - In OnStart, initialize the PowerShell executor and start your main logic
   - In OnStop, clean up resources and stop any running operations
5. Add an installer for the Windows service:
   - Right-click on your service class in the designer
   - Select "Add Installer"
   - Configure the installer properties (service name, display name, description, etc.)
6. Build the project to generate OrchestratorAgent.exe

## 4. Post-Build Steps

1. Copy `NativeLib.dll` to the same directory as `OrchestratorAgent.exe`
2. Copy any other dependent assemblies or configuration files

## 5. Testing

1. Use the Visual Studio Developer Command Prompt to install and test the service:
   - Install: `installutil OrchestratorAgent.exe`
   - Start: `net start OrchestratorAgentService` (use your actual service name)
   - Stop: `net stop OrchestratorAgentService`
   - Uninstall: `installutil /u OrchestratorAgent.exe`

## 6. Packaging

1. Create a deployment package including:
   - OrchestratorAgent.exe
   - NativeLib.dll
   - Any other dependent assemblies or DLLs
   - Configuration files
   - Installation scripts or instructions

## 7. Deployment

1. Copy the deployment package to the target machine
2. Run the installation script or use installutil to install the service
3. Configure any necessary settings
4. Start the service

Remember to thoroughly test the service in a controlled environment before deploying to production. Also, ensure that all necessary error handling and logging are in place to facilitate troubleshooting in a production environment.