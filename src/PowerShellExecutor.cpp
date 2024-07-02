#include "PowerShellExecutor.h"
#include "ProcessUtil.h"
#include "SettingsService.h"
#include "LogUtil.h"
#include "StringUtil.h"

using namespace msclr::interop;
using namespace System::Globalization;
using namespace System::IO;

// Assuming this is the structure of your Callbacks class
struct Callbacks
{
    void (*LogCallback)(const char *, int);
};

PowerShellExecutor::PowerShellExecutor(IntPtr callbacks)
    : callbacks(callbacks), verboseLinesProcessed(0), warningLinesProcessed(0),
      errorLinesProcessed(0), informationLinesProcessed(0), activityLogCounter(0),
      activityLogThreshold(1000)
{
}

void PowerShellExecutor::SendLog(String ^ logOutput, LogOutputType logType)
{
    if (activityLogCounter > activityLogThreshold)
    {
        auto callbacksPtr = static_cast<Callbacks *>(callbacks.ToPointer());
        std::string message = "ActivityLogthresholdexceeded";
        callbacksPtr->LogCallback(message.c_str(), static_cast<int>(LogOutputType::Error));
        throw gcnew Exception("Activity Log threshold exceeded.");
    }

    auto callbacksPtr = static_cast<Callbacks *>(callbacks.ToPointer());
    std::string utf8LogOutput = marshal_as<std::string>(logOutput);
    callbacksPtr->LogCallback(utf8LogOutput.c_str(), static_cast<int>(logType));

    activityLogCounter++;
}

void PowershellExecutor::Debug_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    try
    {
        String ^ text = "";
        int num = 0;
        if (num < safe_cast<PSDataCollection<DebugRecord ^> ^>(sender)->Count)
        {
            do
            {
                DebugRecord ^ debugRecord = (*static_cast<PSDataCollection<DebugRecord ^> ^>(sender))[num];
                text += debugRecord->Message;
                num++;
            } while (num < safe_cast<PSDataCollection<DebugRecord ^> ^>(sender)->Count);
        }
        this->SendLog(text, LogOutputType::Debug);
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "Debug_DataAdded");
    }
}

void PowershellExecutor::Progress_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    try
    {
        auto progressRecords = safe_cast<PSDataCollection<ProgressRecord ^> ^>(sender);
        for (int i = 0; i < progressRecords->Count; i++)
        {
            auto progressRecord = progressRecords[i];
            String ^ activity = progressRecord->Activity;
            String ^ statusDescription = progressRecord->StatusDescription;
            Int32 percentComplete = progressRecord->PercentComplete;

            String ^ message = String::Format("Activity: {0}, StatusDescription: {1}, PercentComplete: {2}%",
                                              activity, statusDescription, percentComplete);
            this->SendLog(message, LogOutputType::Information);
        }
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "Progress_DataAdded");
    }
}

void PowerShellExecutor::Error_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    PSDataCollection<ErrorRecord ^> ^ errorRecords = safe_cast<PSDataCollection<ErrorRecord ^> ^>(sender);
    for (int i = 0; i < errorRecords->Count; i++)
    {
        ErrorRecord ^ errorRecord = errorRecords[i];
        String ^ errorMessage = errorRecord->ToString();
        SendLog(errorMessage, LogOutputType::Error);

        if (errorRecord->Exception != nullptr)
        {
            String ^ exceptionMessage = errorRecord->Exception->ToString();
            SendLog(exceptionMessage, LogOutputType::Error);

            WebException ^ webEx = dynamic_cast<WebException ^>(errorRecord->Exception->InnerException);
            if (webEx != nullptr)
            {
                try
                {
                    String ^ responseText = gcnew StreamReader(webEx->Response->GetResponseStream())->ReadToEnd();
                    SendLog("Web Exception Details: " + responseText, LogOutputType::Error);
                }
                catch (Exception ^ ex)
                {
                    SendLog("Error reading web exception details: " + ex->ToString(), LogOutputType::Error);
                }
            }
        }
    }
}

void PowerShellExecutor::Verbose_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    PSDataCollection<VerboseRecord ^> ^ verboseRecords = safe_cast<PSDataCollection<VerboseRecord ^> ^>(sender);
    String ^ text = "";
    for (int i = verboseLinesProcessed; i < verboseRecords->Count; i++)
    {
        VerboseRecord ^ verboseRecord = verboseRecords[i];
        text += verboseRecord->Message;
        verboseLinesProcessed++;
    }
    SendLog(text, LogOutputType::Verbose);
}

void PowerShellExecutor::Warning_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    PSDataCollection<WarningRecord ^> ^ warningRecords = safe_cast<PSDataCollection<WarningRecord ^> ^>(sender);
    String ^ text = "";
    for (int i = warningLinesProcessed; i < warningRecords->Count; i++)
    {
        WarningRecord ^ warningRecord = warningRecords[i];
        text += warningRecord->Message;
        warningLinesProcessed++;
    }
    SendLog(text, LogOutputType::Warning);
}

void PowershellExecutor::Information_DataAdded(Object ^ sender, DataAddedEventArgs ^ e)
{
    try
    {
        String ^ text = "";
        int num = this->informationLinesProcessed;
        if (num < ((PSDataCollection<InformationRecord ^> ^) sender)->Count)
        {
            do
            {
                InformationRecord ^ informationRecord = ((PSDataCollection<InformationRecord ^> ^) sender)[num];
                text += informationRecord->MessageData;
                this->informationLinesProcessed++;
                num++;
            } while (num < ((PSDataCollection<InformationRecord ^> ^) sender)->Count);
        }
        this->SendLog(text, LogOutputType::Information);
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "Information_DataAdded");
    }
}

void PowershellExecutor::OnOutputDataReceived(Object ^ sender, DataReceivedEventArgs ^ e)
{
    try
    {
        String ^ data = e->Data;
        if (data->Length > 0)
        {
            this->SendLog(data, LogOutputType::Information);
        }
        GC::KeepAlive(this);
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "OnOutputDataReceived");
    }
}

void PowershellExecutor::Host_OnInformation(String ^ information)
{
    try
    {
        String ^ text = information->Trim();
        if (text->Length > 0)
        {
            this->SendLog(text, LogOutputType::Information);
        }
        GC::KeepAlive(this);
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "Host_OnInformation");
    }
}

void PowershellExecutor::HandleInformation(String ^ logOutput)
{
    try
    {
        if (logOutput->Length > 0)
        {
            this->SendLog(logOutput, LogOutputType::Information);
        }
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "HandleInformation");
    }
}

void PowershellExecutor::BindEvents(PowerShell ^ _ps, DefaultHost ^ host)
{
    try
    {
        PSDataCollection<DebugRecord ^> ^ debug = _ps->Streams->Debug;
        EventHandler<DataAddedEventArgs ^> ^ eventHandler = gcnew EventHandler<DataAddedEventArgs ^>(this, &PowershellExecutor::Debug_DataAdded);
        debug->DataAdded += eventHandler;

        PSDataCollection<ErrorRecord ^> ^ error = _ps->Streams->Error;
        EventHandler<DataAddedEventArgs ^> ^ eventHandler2 = gcnew EventHandler<DataAddedEventArgs ^>(this, &PowershellExecutor::Error_DataAdded);
        error->DataAdded += eventHandler2;

        PSDataCollection<ProgressRecord ^> ^ progress = _ps->Streams->Progress;
        EventHandler<DataAddedEventArgs ^> ^ eventHandler3 = gcnew EventHandler<DataAddedEventArgs ^>(this, &PowershellExecutor::Progress_DataAdded);
        progress->DataAdded += eventHandler3;

        PropertyInfo ^ property = _ps->GetType()->GetProperty("Information");
        if (property != nullptr)
        {
            Object ^ value = property->GetValue(_ps->Streams, nullptr);
            EventInfo ^ eventInfo = value->GetType()->GetEvent("DataAdded");
            MethodInfo ^ methodInfo = this->GetType()->GetMethod("Information_DataAdded", BindingFlags::Instance | BindingFlags::NonPublic);
            Delegate ^ delegateInfo = Delegate::CreateDelegate(eventInfo->EventHandlerType, this, methodInfo);
            eventInfo->AddEventHandler(value, delegateInfo);
        }
        else
        {
            host->OnInformation += gcnew Action<String ^>(this, &PowershellExecutor::Host_OnInformation);
        }

        PSDataCollection<VerboseRecord ^> ^ verbose = _ps->Streams->Verbose;
        EventHandler<DataAddedEventArgs ^> ^ eventHandler4 = gcnew EventHandler<DataAddedEventArgs ^>(this, &PowershellExecutor::Verbose_DataAdded);
        verbose->DataAdded += eventHandler4;

        PSDataCollection<WarningRecord ^> ^ warning = _ps->Streams->Warning;
        EventHandler<DataAddedEventArgs ^> ^ eventHandler5 = gcnew EventHandler<DataAddedEventArgs ^>(this, &PowershellExecutor::Warning_DataAdded);
        warning->DataAdded += eventHandler5;

        GC::KeepAlive(this);
    }
    catch (Exception ^ ex)
    {
        LogManagedException(ex, "BindEvents");
    }
}

PowerShellExecutionResult *PowerShellExecutor::ExecutePowerShell(PowerShellExecutionResult *result, String ^ script, bool isInlinePowershell)
{
    PowerShellExecutionResult ^ powerShellExecutionResult = gcnew PowerShellExecutionResult();
    try
    {
        LogUtil::LogInfo("PowerShellExecuteBegin", false);

        Runspace ^ runspace = nullptr;
        PowerShell ^ powerShell = nullptr;
        PSObject ^ psObject = nullptr;
        PSObject ^ psObject2 = nullptr;
        PSDataCollection<Object ^> ^ psDataCollection = nullptr;
        PSDataCollection<Object ^> ^ psDataCollection2 = nullptr;

        try
        {
            ProcessUtil ^ processUtil = gcnew ProcessUtil();
            SettingsService ^ settingsService = gcnew SettingsService();
            bool isActivityNode = SettingsService::GetIsActivityNode(settingsService) != nullptr;

            // OpenPowerShellRunspace
            LogUtil::LogInfo("OpenPowerShellRunspace", false);
            DefaultHost ^ defaultHost = gcnew DefaultHost(CultureInfo::CurrentCulture, CultureInfo::CurrentUICulture);
            runspace = RunspaceFactory::CreateRunspace(defaultHost);
            runspace->Open();
            powerShell = PowerShell::Create();
            powerShell->Runspace = runspace;
            BindEvents(powerShell, defaultHost);

            String ^ executablePath = ProcessUtil::GetExecutableDirectoryPath();
            String ^ runtimeDllPath = Path::Combine(executablePath, "CB.PowerShellRuntimeExtensions20.dll");

            // Load custom PowerShell runtime extensions
            String ^ initScript = String::Format(R"(
                Add-Type -Path '{0}'
                function ConvertFrom-Json20([object] $inputObject) {{
                    $err = $null
                    return [CB.PowerShellRuntimeExtensions.JsonObject]::ConvertFromJson($inputObject, $false, 4, [ref]$err)
                }}
                function ConvertTo-Json20([object] $inputObject, $depth = 5) {{
                    $ctx = New-Object CB.PowerShellRuntimeExtensions.ConvertToJsonContext($depth, $false, $false, 'Default')
                    return [CB.PowerShellRuntimeExtensions.JsonObject]::ConvertToJson($inputObject, [ref]$ctx)
                }}
                if ((Get-Command 'ConvertTo-Json20' -ErrorAction SilentlyContinue) -eq $null) {{ New-Alias -Name 'ConvertTo-JSON' -Value 'ConvertTo-Json20' }}
                if ((Get-Command 'ConvertFrom-Json20' -ErrorAction SilentlyContinue) -eq $null) {{ New-Alias -Name 'ConvertFrom-JSON' -Value 'ConvertFrom-Json20' }}
            )",
                                                 runtimeDllPath);

            powerShell->AddScript(initScript);
            powerShell->Invoke();

            // Deserialize script variables (you'll need to implement this part)
            String ^ deserializedScript = DeserializeScriptVariables(script);

            powerShell->AddScript(deserializedScript, true);
            PSInvocationSettings ^ psInvocationSettings = gcnew PSInvocationSettings();
            psInvocationSettings->Host = defaultHost;
            psDataCollection2 = gcnew PSDataCollection<Object ^>();
            Collection<PSObject ^> ^ psOutput = powerShell->Invoke<Object ^>(nullptr, psDataCollection2, psInvocationSettings);

            if (psDataCollection2->Count == 0 && (!isActivityNode || !isInlinePowershell))
            {
                throw gcnew Exception("Activity did not return a result and/or failed while executing");
            }

            if (psDataCollection2->Count > 1)
            {
                throw gcnew Exception("Activity returned more than one result. See below for details");
            }

            if (psDataCollection2->Count > 0)
            {
                psObject = gcnew PSObject();
                Type ^ type = psDataCollection2[0]->GetType();
                Type ^ type2 = psObject->GetType();
                if (type != type2)
                {
                    throw gcnew Exception("Activity did not return valid result. Must return object.");
                }
                psObject2 = psDataCollection2[0];
                String ^ outputText = psObject2->ToString();
                std::string outputStr = marshal_as<std::string>(outputText);
                result->Output = new char[outputStr.length() + 1];
                strcpy_s(result->Output, outputStr.length() + 1, outputStr.c_str());
            }

            result->Success = true;
        }
        catch (Exception ^ ex)
        {
            result->Success = false;
            std::string errorStr = marshal_as<std::string>(ex->ToString());
            result->Output = new char[errorStr.length() + 1];
            strcpy_s(result->Output, errorStr.length() + 1, errorStr.c_str());
        }
        finally
        {
            if (psDataCollection != nullptr)
                psDataCollection->Dispose();
            if (psDataCollection2 != nullptr)
                psDataCollection2->Dispose();
            if (runspace != nullptr)
                runspace->Dispose();
            if (powerShell != nullptr)
                powerShell->Dispose();
            if (psObject2 != nullptr && dynamic_cast<IDisposable ^>(psObject2) != nullptr)
                safe_cast<IDisposable ^>(psObject2)->Dispose();
            if (psObject != nullptr && dynamic_cast<IDisposable ^>(psObject) != nullptr)
                safe_cast<IDisposable ^>(psObject)->Dispose();
        }

        GC::KeepAlive(this);
    }
    catch (Exception ^ ex)
    {
        // Handle any unexpected exceptions
        result->Success = false;
        std::string errorStr = marshal_as<std::string>(ex->ToString());
        result->Output = new char[errorStr.length() + 1];
        strcpy_s(result->Output, errorStr.length() + 1, errorStr.c_str());
    }

    return result;
}

// Implement DeserializeScriptVariables method
String ^ PowerShellExecutor::DeserializeScriptVariables(String ^ script)
{
    // TODO: Implement your deserialization logic here
    // This is a placeholder implementation
    return script;
}