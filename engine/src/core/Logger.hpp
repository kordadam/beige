#pragma once

#include "../Defines.hpp"
#include "../platform/Platform.hpp"

#include <string>
#include <cstdint>
#include <map>
#include <fstream>

namespace beige {
namespace core {

class BEIGE_API Logger final {
public:
    Logger() = delete;
    ~Logger() = delete;

    static auto reportAssertionFailure(
        const std::string& expression,
        const std::string& message,
        const std::string& file,
        const uint32_t line
    ) -> void;

    static auto trace(const std::string& message) -> void;
    static auto debug(const std::string& message) -> void;
    static auto info(const std::string& message) -> void;
    static auto warn(const std::string& message) -> void;
    static auto error(const std::string& message) -> void;
    static auto fatal(const std::string& message) -> void;

private:
    enum class Level : uint32_t {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    static std::fstream m_logFile;
    static class Initializer {
    public:
        Initializer() {
            m_logFile.open("console.log", std::fstream::out);
        }
        ~Initializer() {
            m_logFile.close();
        }
    } m_initializer;

    static const std::map<Level, platform::ConsoleColor> m_levelConsoleColorMap;

    friend auto operator<< (std::ostream& outputStream, const Level level)->std::ostream&;

    static auto writeLog(const Level level, const std::string& message) -> void;
    static auto appendToLogFile(const std::string& message) -> void;
};

} // namespace core
} // namespace beige
