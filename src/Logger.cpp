#include "Logger.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);  // auto-unlock at end of scope

    std::ofstream file("server.log", std::ios::app);
    if (file.is_open()) {
        file << "[" << getCurrentTime() << "] " << message << "\n";
    }
}

std::string Logger::getCurrentTime() {
    time_t now = time(nullptr);
    struct tm timeinfo;

#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    localtime_r(&now, &timeinfo);
#endif

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buf);
}
