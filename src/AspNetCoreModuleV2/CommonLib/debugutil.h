// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once
#include "stdafx.h"
#include "dbgutil.h"

#define ASPNETCORE_DEBUG_FLAG_INFO          DEBUG_FLAG_INFO
#define ASPNETCORE_DEBUG_FLAG_WARNING       DEBUG_FLAG_WARN
#define ASPNETCORE_DEBUG_FLAG_ERROR         DEBUG_FLAG_ERROR
#define ASPNETCORE_DEBUG_FLAG_CONSOLE       0x00000008

static
inline
VOID
DebugInitialize()
{
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\IIS Extensions\\IIS AspNetCore Module\\Parameters",
            0,
            KEY_READ,
            &hKey) == NO_ERROR)
    {
        DWORD dwType;
        DWORD dwData;
        DWORD cbData;

        cbData = sizeof(dwData);
        if ((RegQueryValueEx(hKey,
            L"DebugFlags",
            NULL,
            &dwType,
            (LPBYTE)&dwData,
            &cbData) == NO_ERROR) &&
            (dwType == REG_DWORD))
        {
            DEBUG_FLAGS_VAR = dwData;
        }

        RegCloseKey(hKey);
    }

    // We expect single digit value and a null char
    const size_t environmentVariableValueSize = 2;
    std::wstring environmentVariableValue(environmentVariableValueSize, '\0');

    if (GetEnvironmentVariable(L"ASPNETCORE_MODULE_DEBUG", environmentVariableValue.data(), environmentVariableValueSize) == environmentVariableValueSize - 1)
    {
        try
        {
            const auto value = std::stoi(environmentVariableValue);

            if (value >= 1) DEBUG_FLAGS_VAR |= ASPNETCORE_DEBUG_FLAG_ERROR;
            if (value >= 2) DEBUG_FLAGS_VAR |= ASPNETCORE_DEBUG_FLAG_WARNING;
            if (value >= 3) DEBUG_FLAGS_VAR |= ASPNETCORE_DEBUG_FLAG_INFO;
            if (value >= 4) DEBUG_FLAGS_VAR |= ASPNETCORE_DEBUG_FLAG_CONSOLE;
        }
        catch (...)
        {
            // ignore
        }
    }
}

static
BOOL
IsEnabled(
    DWORD   dwFlag
    )
{
    return ( dwFlag & DEBUG_FLAGS_VAR );
}

static
VOID
DebugPrint(
    DWORD   dwFlag,
    LPCSTR  szString
    )
{
    STACK_STRA (strOutput, 256);
    HRESULT  hr = S_OK;

    if ( IsEnabled( dwFlag ) )
    {
        hr = strOutput.SafeSnprintf(
            "[%s] %s\r\n",
            DEBUG_LABEL_VAR, szString );

        if (FAILED (hr))
        {
            return;
        }

        OutputDebugStringA( strOutput.QueryStr() );

        if (IsEnabled(ASPNETCORE_DEBUG_FLAG_CONSOLE))
        {
            fputs(strOutput.QueryStr(), stdout);
        }
    }
}

static
VOID
DebugPrintf(
DWORD   dwFlag,
LPCSTR  szFormat,
...
)
{
    STACK_STRA (strCooked,256);

    va_list  args;
    HRESULT hr = S_OK;

    if ( IsEnabled( dwFlag ) )
    {
        va_start( args, szFormat );

        hr = strCooked.SafeVsnprintf(szFormat, args );

        va_end( args );

        if (FAILED (hr))
        {
            return;
        }

        DebugPrint( dwFlag, strCooked.QueryStr() );
    }
}

