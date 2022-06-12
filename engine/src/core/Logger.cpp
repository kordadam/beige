#include "Logger.hpp"

#include <iostream>
#include <sstream>

namespace beige {
namespace core {

#define LOG_TRACE_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_WARN_ENABLED 1

const std::map<Logger::Level, platform::ConsoleColor> Logger::m_levelConsoleColorMap{
    { Level::Trace, platform::ConsoleColor::Gray },
    { Level::Debug, platform::ConsoleColor::Cyan },
    { Level::Info, platform::ConsoleColor::Green },
    { Level::Warn, platform::ConsoleColor::Yellow },
    { Level::Error, platform::ConsoleColor::Red },
    { Level::Fatal, platform::ConsoleColor::RedBackground }
};

auto Logger::reportAssertionFailure(
    const std::string& expression,
    const std::string& message,
    const std::string& file,
    const uint32_t line
) -> void {
    const std::string assertionFailureMessage {
        "Assertion failure: " +
        expression +
        ", message: " +
        message +
        ", in file: " +
        file +
        ", line: " +
        std::to_string(line)
    };

    fatal(assertionFailureMessage);
}

auto Logger::trace(const std::string& message) -> void {
#if LOG_TRACE_ENABLED == 1
    writeLog(Level::Trace, message);
#endif
}

auto Logger::debug(const std::string& message) -> void {
#if LOG_DEBUG_ENABLED == 1
    writeLog(Level::Debug, message);
#endif
}

auto Logger::info(const std::string& message) -> void {
#if LOG_INFO_ENABLED == 1
    writeLog(Level::Info, message);
#endif
}

auto Logger::warn(const std::string& message) -> void {
#if LOG_WARN_ENABLED == 1
    writeLog(Level::Warn, message);
#endif
}

auto Logger::error(const std::string& message) -> void {
    writeLog(Level::Error, message);
}

auto Logger::fatal(const std::string& message) -> void {
    writeLog(Level::Fatal, message);
}

auto operator<< (std::ostream& outputStream, const Logger::Level level) -> std::ostream& {
    switch (level) {
    case Logger::Level::Trace: return outputStream << "[TRACE]";
    case Logger::Level::Debug: return outputStream << "[DEBUG]";
    case Logger::Level::Info: return outputStream << "[INFO]";
    case Logger::Level::Warn: return outputStream << "[WARN]";
    case Logger::Level::Error: return outputStream << "[ERROR]";
    case Logger::Level::Fatal: return outputStream << "[FATAL]";
    }
}

auto Logger::writeLog(const Level level, const std::string& message) -> void {
    std::stringstream consoleMessage;
    consoleMessage << level << " " << message;
    platform::Platform::consoleWrite(consoleMessage.str(), m_levelConsoleColorMap.at(level));
}

} // namespace core
} // namespace beige
