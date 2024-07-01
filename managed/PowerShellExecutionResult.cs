namespace PowerShellExecutorNamespace
{
    public class PowerShellExecutionResult
    {
        public bool Success { get; set; }
        public string Output { get; set; }
        public string ErrorMessage { get; set; }
    }
}