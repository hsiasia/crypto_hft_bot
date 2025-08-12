// Logger declaration

#pragma once
#include <string>

class Logger {
private:
    class Impl;
    Impl* pImpl;

public:
    Logger();
    ~Logger();

    void log(const std::string& msg);
};