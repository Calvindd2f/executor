#define NATIVELIB_API __declspec(dllimport)
#endif

#include <string>

struct Callbacks;
struct PowerShellExecutionResult;

extern "C"
{
    // Existing function
    NATIVELIB_API void SendLog(Callbacks *callbacks, const char *message, int logType);

    // New functions
    NATIVELIB_API void *CreatePowerShellExecutor(Callbacks *callbacks);
    NATIVELIB_API void DestroyPowerShellExecutor(void *executor);
    NATIVELIB_API PowerShellExecutionResult ExecutePowerShell(void *executor, const char *script, bool isInlinePowershell);
    NATIVELIB_API void FreePowerShellExecutionResult(PowerShellExecutionResult *result);
    NATIVELIB_API const char *GetExecutableDirectoryPath();
    NATIVELIB_API bool IsActivityNode();
}

struct Callbacks
{
    void (*LogCallback)(const char *message, int logType);
};

struct PowerShellExecutionResult
{
    bool Success;
    char *Output;
    char *ErrorMessage;
};