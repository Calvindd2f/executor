// include/PowerShellExecutorWrapper.h
#pragma once

#using < System.dll>
#using < System.Collections.dll>
#include <msclr\marshal_cppstd.h>
#include "PowerShellExecutor.h"

using namespace System;
using namespace System::Collections::Generic;

public
ref class PowerShellExecutorWrapper
{
public:
    PowerShellExecutorWrapper();
    ~PowerShellExecutorWrapper();
    !PowerShellExecutorWrapper();

    String ^ ExecuteScript(String ^ script, Dictionary<String ^, String ^> ^ parameters);

private:
    PowerShellExecutor *m_pExecutor;
};