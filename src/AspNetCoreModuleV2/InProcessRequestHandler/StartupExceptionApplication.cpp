// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "precomp.hxx"

#include "StartupExceptionApplication.h"

VOID StartupExceptionApplication::ShutDown()
{
}

HRESULT StartupExceptionApplication::CreateHandler(IHttpContext *pHttpContext, IREQUEST_HANDLER ** pRequestHandler)
{
    *pRequestHandler = new StartupExceptionHandler(pHttpContext, m_disableLogs, this);
    return S_OK;
}
