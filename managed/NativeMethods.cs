using System;
using System.Runtime.InteropServices;

namespace PowerShellExecutorNamespace
{
    public static class NativeMethods
    {
        [DllImport("NativeLib.dll")]
        public static extern void SendLog(IntPtr callbacks, string message, int logType);

        [DllImport("NativeLib.dll")]
        public static extern IntPtr CreatePowerShellExecutor(IntPtr callbacks);

        [DllImport("NativeLib.dll")]
        public static extern void DestroyPowerShellExecutor(IntPtr executor);

        [DllImport("NativeLib.dll")]
        public static extern PowerShellExecutionResult ExecutePowerShell(IntPtr executor, string script, [MarshalAs(UnmanagedType.U1)] bool isInlinePowershell);

        [DllImport("NativeLib.dll")]
        public static extern void FreePowerShellExecutionResult(ref PowerShellExecutionResult result);

        [DllImport("NativeLib.dll")]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string GetExecutableDirectoryPath();

        [DllImport("NativeLib.dll")]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool IsActivityNode();
    }
}

// Don't forget to free the memory when you're done:
// Marshal.FreeHGlobal(callbacksPtr);