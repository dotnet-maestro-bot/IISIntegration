#pragma once

#define EVENTLOG(log, name, ...) UTILITY::LogEventF(log, ASPNETCORE_EVENT_ ## name ## _LEVEL, ASPNETCORE_EVENT_ ## name, ASPNETCORE_EVENT_ ## name ## _MSG, __VA_ARGS__)
