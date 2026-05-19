#pragma once

#include <chrono>
#include <format>
#include <string>
#include <thread>
#include <iostream>

#include <logger.hpp>

enum class LogLevel {
    Debug = 0,
    Info,
    Warn,
    Error
};

inline std::string to_string(const LogLevel level) {
    switch (level) {
        case LogLevel::Debug: {
            return "debug";
        }
        case LogLevel::Info: {
            return "info";
        }
        case LogLevel::Warn: {
            return "warn";
        }
        case LogLevel::Error: {
            return "error";
        }
    }
    return "?";
}

inline const auto program_start_time = std::chrono::steady_clock::now();


class LogAdditionInfo {
    public:
        std::string addition_str;
        LogAdditionInfo() noexcept {
            const auto curr_time = std::chrono::steady_clock::now();
            // const auto local_time = std::chrono::zoned_time{std::chrono::current_zone(), time_info};

            const auto thread_id = std::this_thread::get_id();

            addition_str = std::format("[thread-{}] <{:%H:%M:%S}>", thread_id, curr_time - program_start_time);
        }
};


using LoggerType = Logger<LogLevel, LogAdditionInfo>;

namespace global_logger {
    static LogLevel minLevel{LogLevel::Info};

    inline bool level_filter(const LoggerType::LogRecord& record) noexcept {
        // if (record.level >= minLevel) return false;
        return true;
    }

    inline void console_handler(const LoggerType::LogRecord& record, const std::string& message) noexcept {
        std::cout << message << std::endl;
    }

    inline std::string log_formatter(const LoggerType::LogRecord& record) noexcept {
        return std::format("{} [{}]: {}", record.additionInfo.addition_str, to_string(record.level), record.message);
    }
}

inline LoggerType glog = LoggerType::builder()
    .appendFilter(global_logger::level_filter)
    .appendHandler(global_logger::console_handler)
    .formatter(global_logger::log_formatter)
    .build();
