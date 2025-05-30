//
// Created by Akshat Jain on 20/04/25.
//

#ifndef LOGGING_H
#define LOGGING_H

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/ostr.h>

namespace logging {

    // Singleton-style accessor for the logger
    inline std::shared_ptr<spdlog::logger> getLogger() {
        static std::shared_ptr<spdlog::logger> logger = []() {
            auto logger = spdlog::basic_logger_mt("file_logger", "application.log");
            logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
            logger->set_level(spdlog::level::debug);
            logger->flush_on(spdlog::level::debug);
            return logger;
        }();
        return logger;
    }

    // Simple wrapper struct to make calls like log.info("...")
    struct LoggerWrapper {
        template <typename... Args>
        void info(fmt::format_string<Args...> fmt, Args&&... args) const {
            getLogger()->info(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> fmt, Args&&... args) const {
            getLogger()->warn(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args&&... args) const {
            getLogger()->error(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void critical(fmt::format_string<Args...> fmt, Args&&... args) const {
            getLogger()->critical(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debug(fmt::format_string<Args...> fmt, Args&&... args) const {
            getLogger()->debug(fmt, std::forward<Args>(args)...);
        }
    };

    inline const LoggerWrapper log;

}  // namespace logging

#endif  // LOGGING_H
