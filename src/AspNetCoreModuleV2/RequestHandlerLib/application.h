// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <httpserv.h>

#include "iapplication.h"
#include "exceptions.h"
#include "requesthandler_config.h"
#include "filewatcher.h"
#include "..\CommonLib\aspnetcore_msg.h"
#include "..\CommonLib\resources.h"
#include "utility.h"

extern "C" HANDLE        g_hEventLog;

class APPLICATION : public IAPPLICATION
{

public:
    APPLICATION_STATUS
    QueryStatus() override
    {
        return m_status;
    }

    APPLICATION()
        : m_cRefs(1)
    {
    }

    VOID
    ReferenceApplication() override
    {
        InterlockedIncrement(&m_cRefs);
    }

    VOID
    DereferenceApplication() override
    {
        DBG_ASSERT(m_cRefs != 0);

        if (InterlockedDecrement(&m_cRefs) == 0)
        {
            delete this;
        }
    }

protected:
    volatile APPLICATION_STATUS     m_status = APPLICATION_STATUS::UNKNOWN;
private:
    // Non-copyable
    APPLICATION(const APPLICATION&) = delete;
    const APPLICATION& operator=(const APPLICATION&) = delete;

    mutable LONG                    m_cRefs;
};

class AppOfflineApplication: public APPLICATION
{
public:
    AppOfflineApplication(const IHttpApplication& application)
        : m_applicationPath(application.GetApplicationPhysicalPath()),
        m_fileWatcher(nullptr),
        m_fileWatcherEntry(nullptr)
    {
    }

    ~AppOfflineApplication() override = default;
    
    HRESULT
    StartMonitoringAppOffline()
    {
        LOG_INFOF("Starting app_offline monitoring in application %S", m_applicationPath.c_str());
        HRESULT hr = StartMonitoringAppOflineImpl();

        if(FAILED_LOG(hr))
        {
            UTILITY::LogEventF(g_hEventLog,
                EVENTLOG_WARNING_TYPE,
                ASPNETCORE_EVENT_MONITOR_APPOFFLINE_ERROR,
                ASPNETCORE_EVENT_MONITOR_APPOFFLINE_ERROR_MSG,
                m_applicationPath.c_str(),
                hr);
        }

        return hr;
    }

    HRESULT
    StartMonitoringAppOflineImpl()
    {
        if (m_fileWatcher)
        {
            RETURN_IF_FAILED(E_UNEXPECTED);
        }

        m_fileWatcher = std::make_unique<FILE_WATCHER>();
        RETURN_IF_FAILED(m_fileWatcher->Create());
        m_fileWatcherEntry.reset(new FILE_WATCHER_ENTRY(m_fileWatcher.get()));
        RETURN_IF_FAILED(m_fileWatcherEntry->Create(m_applicationPath.c_str(), L"app_offline.htm", this, NULL));

        return S_OK;
    }

    VOID
    OnAppOffline() override
    {
        LOG_INFOF("Received app_offline notification in application %S", m_applicationPath.c_str());
        m_fileWatcherEntry->StopMonitor();
        m_status = APPLICATION_STATUS::SHUTDOWN;
        UTILITY::LogEventF(g_hEventLog,
            EVENTLOG_INFORMATION_TYPE,
            ASPNETCORE_EVENT_RECYCLE_APPOFFLINE,
            ASPNETCORE_EVENT_RECYCLE_APPOFFLINE_MSG,
            m_applicationPath.c_str());

        Recycle();
    }

private:
    std::wstring                                 m_applicationPath;
    std::unique_ptr<FILE_WATCHER>                m_fileWatcher;
    std::unique_ptr<FILE_WATCHER_ENTRY, FILE_WATCHER_ENTRY_DELETER>  m_fileWatcherEntry;
};
