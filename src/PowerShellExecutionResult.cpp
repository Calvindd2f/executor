#include "PowerShellExecutionResult.h"
#include <msclr/marshal.h>
#include <string>

using namespace msclr::interop;

PowerShellExecutionResult::PowerShellExecutionResult()
    : _success(false), _output(nullptr), _errorMessage(nullptr)
{
}

PowerShellExecutionResult::~PowerShellExecutionResult()
{
    this->!PowerShellExecutionResult();
}

PowerShellExecutionResult::!PowerShellExecutionResult()
{
    // Cleanup code, if any
}

void PowerShellExecutionResult::ToUnmanaged(PowerShellExecutionResultUnmanaged *result)
{
    if (result == nullptr)
        throw gcnew ArgumentNullException("result");

    result->Success = _success;

    if (_output != nullptr)
    {
        std::string outputStr = marshal_as<std::string>(_output);
        result->Output = new char[outputStr.length() + 1];
        strcpy_s(result->Output, outputStr.length() + 1, outputStr.c_str());
    }
    else
    {
        result->Output = nullptr;
    }

    if (_errorMessage != nullptr)
    {
        std::string errorStr = marshal_as<std::string>(_errorMessage);
        result->ErrorMessage = new char[errorStr.length() + 1];
        strcpy_s(result->ErrorMessage, errorStr.length() + 1, errorStr.c_str());
    }
    else
    {
        result->ErrorMessage = nullptr;
    }
}

void PowerShellExecutionResult::FromUnmanaged(const PowerShellExecutionResultUnmanaged *result)
{
    if (result == nullptr)
        throw gcnew ArgumentNullException("result");

    _success = result->Success;
    _output = result->Output != nullptr ? gcnew String(result->Output) : nullptr;
    _errorMessage = result->ErrorMessage != nullptr ? gcnew String(result->ErrorMessage) : nullptr;
}