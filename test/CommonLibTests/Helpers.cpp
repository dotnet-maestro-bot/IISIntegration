// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "stdafx.h"

void
Helpers::DeleteDirectory(std::wstring directory)
{
    std::experimental::filesystem::remove_all(directory);
}

std::wstring
Helpers::ReadFileContent(std::wstring file)
{
    std::wcout << file << std::endl;

    std::fstream t(file);
    std::stringstream buffer;
    buffer << t.rdbuf();

    int nChars = MultiByteToWideChar(CP_ACP,  0, buffer.str().c_str(), -1, NULL, 0);

    LPWSTR pwzName = new WCHAR[nChars];
    MultiByteToWideChar(CP_UTF8, 0, buffer.str().c_str(), -1, pwzName, nChars);

    std::wstring retVal(pwzName);

    delete pwzName;

    return retVal;
}

TempDirectory::TempDirectory()
{
    UUID uuid;
    UuidCreate(&uuid);
    RPC_CSTR szUuid = NULL;
    if (UuidToStringA(&uuid, &szUuid) == RPC_S_OK)
    {
        m_path = std::experimental::filesystem::temp_directory_path() / szUuid;
        RpcStringFreeA(&szUuid);
        return;
    }
    throw std::exception("Cannot create temp directory");
}

TempDirectory::~TempDirectory()
{
    std::experimental::filesystem::remove_all(m_path);
}
