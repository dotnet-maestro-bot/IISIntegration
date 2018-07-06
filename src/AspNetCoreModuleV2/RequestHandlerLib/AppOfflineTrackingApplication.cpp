// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#include "stdafx.h"
#include "AppOfflineTrackingApplication.h"
#include "EventLog.h"
#include "exceptions.h"

HRESULT AppOfflineTrackingApplication::StartMonitoringAppOffline()
{
    LOG_INFOF("Starting app_offline monitoring in application %S", m_applicationPath.c_str());
    HRESULT hr = StartMonitoringAppOflineImpl();

    if (FAILED_LOG(hr))
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

HRESULT AppOfflineTrackingApplication::StartMonitoringAppOflineImpl()
{
    if (m_fileWatcher)
    {
        RETURN_IF_FAILED(E_UNEXPECTED);
    }

    m_fileWatcher = std::make_unique<FILE_WATCHER>();
    RETURN_IF_FAILED(m_fileWatcher->Create());
    m_fileWatcherEntry.reset(new FILE_WATCHER_ENTRY(m_fileWatcher.get()));
    RETURN_IF_FAILED(m_fileWatcherEntry->Create(
        m_applicationPath.c_str(),
        L"app_offline.htm",
        std::bind(&AppOfflineTrackingApplication::OnAppOffline, this),
        NULL));

    return S_OK;
}

void AppOfflineTrackingApplication::OnAppOffline()
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
