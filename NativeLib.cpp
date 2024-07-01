#include "NativeLib.h"
#include <stdexcept>
#include <memory>
#include <Windows.h>

// Forward declaration of the PowerShellExecutor class
class PowerShellExecutor;

extern "C"
{

    NATIVELIB_API void SendLog(Callbacks *callbacks, const char *message, int logType)
    {
        if (callbacks == nullptr || callbacks->LogCallback == nullptr || message == nullptr)
        {
            throw std::runtime_error("Invalid parameters in SendLog");
        }
        callbacks->LogCallback(message, logType);
    }

    NATIVELIB_API void *CreatePowerShellExecutor(Callbacks *callbacks)
    {
        if (callbacks == nullptr)
        {
            throw std::runtime_error("Callbacks pointer is null");
        }
        return new PowerShellExecutor(callbacks);
    }

    NATIVELIB_API void DestroyPowerShellExecutor(void *executor)
    {
        if (executor != nullptr)
        {
            delete static_cast<PowerShellExecutor *>(executor);
        }
    }

    NATIVELIB_API PowerShellExecutionResult ExecutePowerShell(void *executor, const char *script, bool isInlinePowershell)
    {
        if (executor == nullptr || script == nullptr)
        {
            throw std::runtime_error("Invalid parameters in ExecutePowerShell");
        }

        PowerShellExecutor *psExecutor = static_cast<PowerShellExecutor *>(executor);
        // Implement the execution logic here
        // This is a placeholder implementation
        PowerShellExecutionResult result;
        result.Success = true;
        result.Output = _strdup("Execution successful");
        result.ErrorMessage = nullptr;
        return result;
    }

    NATIVELIB_API void FreePowerShellExecutionResult(PowerShellExecutionResult *result)
    {
        if (result != nullptr)
        {
            free(result->Output);
            free(result->ErrorMessage);
        }
    }

    NATIVELIB_API const char *GetExecutableDirectoryPath()
    {
        static char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        char *lastBackslash = strrchr(path, '\\');
        if (lastBackslash != nullptr)
        {
            *lastBackslash = '\0';
        }
        return path;
    }

    NATIVELIB_API bool IsActivityNode()
    {
        // Implement the logic to determine if this is an activity node
        // This is a placeholder implementation
        return false;
    }
}

// Actual implementation of PowerShellExecutor class
class PowerShellExecutor
{
public:
    PowerShellExecutor(Callbacks *callbacks) : m_callbacks(callbacks) {}

    // Add methods for PowerShell execution here

private:
    Callbacks *m_callbacks;
};