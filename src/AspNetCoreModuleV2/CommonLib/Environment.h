#pragma once

#include <string>

class Environment
{
public:
	Environment() = delete;
	~Environment() = delete;

    static
    std::wstring ExpandEnvironmentVariables(const std::wstring & str);
};

