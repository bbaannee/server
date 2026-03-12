#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>

// Thread-safe Singleton logger - writes timestamped messages to server.log
class Logger {
public:
    static Logger& getInstance();

    // Non-copyable singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(const std::string& message);

private:
    Logger();
    std::string getCurrentTime();
    std::mutex logMutex;
};

#endif
