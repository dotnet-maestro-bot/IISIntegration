// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "stringa.h"
#include "stringu.h"
#include "HandleWrapper.h"
#include "exceptions.h"

class APP_OFFLINE_HTM
{
public:
    APP_OFFLINE_HTM(LPCWSTR pszPath) : m_cRefs(1)
    {
        m_Path.Copy(pszPath);
    }

    BOOL
    Load(
        VOID
    )
    {
        LARGE_INTEGER   li = { 0 };

        HandleWrapper<InvalidHandleTraits> handle = CreateFile(m_Path.QueryStr(),
                                                               GENERIC_READ,
                                                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                                               NULL,
                                                               OPEN_EXISTING,
                                                               FILE_ATTRIBUTE_NORMAL,
                                                               NULL);

        if (handle == INVALID_HANDLE_VALUE)
        {
            const auto lastErrorIsNotFound = GetLastError() == ERROR_FILE_NOT_FOUND;
            LOG_LAST_ERROR_IF(!lastErrorIsNotFound);
            if (lastErrorIsNotFound)
            {
                return false;
            }

            // This Load() member function is supposed be called only when the change notification event of file creation or file modification happens.
            // If file is currenlty locked exclusively by other processes, we might get INVALID_HANDLE_VALUE even though the file exists. In that case, we should return TRUE here.
            return true;
        }

        if (LOG_LAST_ERROR_IF(!GetFileSizeEx(handle, &li)))
        {
            return true;
        }

        if (li.HighPart != 0)
        {
            // > 4gb file size not supported
            LOG_ERRORF("app_ofline file larger than 4gb");
            return true;
        }

        if (li.LowPart > 0)
        {
            DWORD bytesRead = 0;
            std::unique_ptr<char[]> pszBuff(new CHAR[li.LowPart + 1]);

            if (LOG_LAST_ERROR_IF(!ReadFile(handle, pszBuff.get(), li.LowPart, &bytesRead, NULL)))
            {
                return true;
            }

            LOG_IF_FAILED(m_Contents.Copy(pszBuff.get(), bytesRead));
        }

        return true;
    }

    mutable LONG        m_cRefs;
    STRA                m_Contents;
    STRU                m_Path;
};

