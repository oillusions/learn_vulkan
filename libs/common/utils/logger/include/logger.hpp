#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

/**
 * @brief 日志器本体
 * @details 他似乎不需要详细注释[划掉]
 * @tparam LogLevelEnum 日志等级
 * @tparam AdditionInfo 日志记录附加信息
 */
template<typename LogLevelEnum, typename AdditionInfo>
class Logger {
    static_assert(std::is_enum_v<LogLevelEnum>, "错误: 日志器模板类型[ LogLevelEnum ]必须为枚举类型");
    static_assert(std::is_default_constructible_v<AdditionInfo>, "错误: 日志器模板类型[ AdditionInfo ]必须实现默认构造");
    public:
        using LevelType = LogLevelEnum;
        using AdditionInfoType = AdditionInfo;

        struct LogRecord {
            LogLevelEnum level;
            std::string message;
            AdditionInfo additionInfo;
        };

        /**
         * @brief 日志过滤器基类 | 过滤器包装器
         * @details 他似乎不需要详细注释[划掉]
         */
        class BaseFilter {
            public:
                using FilterType = std::function<bool(const LogRecord&)>;
                BaseFilter() = default;

                template<typename Func, typename = std::enable_if_t<!std::is_base_of_v<BaseFilter, std::decay_t<Func>>>>
                BaseFilter(Func&& filter): _filterFunc(std::forward<Func>(filter)) {};
                virtual ~BaseFilter() = default;

                virtual bool isThrough(const LogRecord& record) {
                    return _filterFunc != nullptr ? _filterFunc(record) : false;
                }
            protected:
                FilterType _filterFunc{};
        };

        /**
         * @brief 日志格式化器基类 | 格式化器包装器
         * @details 他似乎不需要详细注释[划掉]
         */
        class BaseFormatter {
            public:
                using FormatterType = std::function<std::string(const LogRecord&)>;
                BaseFormatter() = default;

                template<typename Func, typename = std::enable_if_t<!std::is_base_of_v<BaseFormatter, std::decay_t<Func>>>>
                BaseFormatter(Func&& formatter): _formatterFunc(std::forward<Func>(formatter)) {};
                virtual ~BaseFormatter() = default;

                virtual std::string format(const LogRecord& record) {
                    return _formatterFunc != nullptr ? _formatterFunc(record) : std::string{};
                }

            private:
                FormatterType _formatterFunc{};
        };

        /**
         * @brief 日志处理器基类 | 处理器包装器
         * @details 他似乎不需要详细注释[划掉]
         */
        class BaseHandler {
            public:
                using HandlerType = std::function<void(const LogRecord&, const std::string&)>;
                BaseHandler() = default;

                template<typename Func, typename = std::enable_if_t<!std::is_base_of_v<BaseHandler, std::decay_t<Func>>>>
                BaseHandler(Func&& handler): _handlerFunc(std::forward<Func>(handler)) {};
                virtual ~BaseHandler() = default;

                virtual void execute(const LogRecord& record, const std::string& str) {
                    if (_handlerFunc != nullptr) {
                        _handlerFunc(record, str);
                    }
                }

            private:
                HandlerType _handlerFunc{};
        };


        /**
         * @brief 日志器构建者
         * @details 他似乎不需要详细注释[划掉]
         */
        class LoggerBuilder {
            public:
                LoggerBuilder() = default;
                ~LoggerBuilder() = default;

                /**
                 * @brief 配置添加日志过滤器
                 * @details 他似乎不需要详细注释[划掉]
                 * @tparam Func 过滤器类型
                 * @param filter 过滤器
                 * @return 构建者引用
                 */
                template<typename Func>
                LoggerBuilder& appendFilter(Func&& filter) {
                    if constexpr (std::is_base_of_v<BaseFilter, std::decay_t<Func>>) {
                        filters.emplace_back(std::make_unique<std::decay_t<Func>>(std::forward<Func>(filter)));
                    } else {
                        filters.emplace_back(std::make_unique<BaseFilter>(std::forward<Func>(filter)));
                    }

                    return *this;
                }

                /**
                 * @brief 配置日志格式化器
                 * @details 他似乎不需要详细注释[划掉]
                 * @tparam Func 格式化类型
                 * @param formatter 格式化器
                 * @return 构建者引用
                 */
                template<typename Func>
                LoggerBuilder& formatter(Func&& formatter) {
                    if constexpr (std::is_base_of_v<BaseFormatter, std::decay_t<Func>>) {
                        _formatter = std::make_unique<std::decay_t<Func>>(std::forward<Func>(formatter));
                    } else {
                        _formatter = std::make_unique<BaseFormatter>(std::forward<Func>(formatter));
                    }
                    return *this;
                }

                /**
                 * @brief 配置添加日志处理器
                 * @tparam Func 处理器类型
                 * @param handler 处理器
                 * @return 构建者引用
                 */
                template<typename Func>
                LoggerBuilder& appendHandler(Func&& handler) {
                    if constexpr (std::is_base_of_v<BaseHandler, std::decay_t<Func>>) {
                        handlers.emplace_back(std::make_unique<std::decay_t<Func>>(std::forward<Func>(handler)));
                    } else {
                        handlers.emplace_back(std::make_unique<BaseHandler>(std::forward<Func>(handler)));
                    }
                    return *this;
                }

                /**
                 * @brief 构建日志器
                 * @details 他似乎不需要详细注释[划掉]
                 * @return 日志器
                 */
                Logger build() {
                    return Logger(std::move(filters), std::move(_formatter), std::move(handlers));
                }

            private:
                std::unique_ptr<BaseFormatter> _formatter;
                std::vector<std::unique_ptr<BaseFilter>> filters;
                std::vector<std::unique_ptr<BaseHandler>> handlers;

                std::queue<LogRecord> _logQueue;


        };

        ~Logger() = default;

        /**
         * @brief 获取构建器
         * @details 他似乎不需要详细注释[划掉]
         * @return 构建器
         */
        static LoggerBuilder builder() {
            return {};
        }

        /**
         * @brief 添加日志
         * @details 他似乎不需要详细注释[划掉]
         * @param level 日志等级
         * @param message 日志消息
         * @param info 日志附加信息
         */
        void log(LogLevelEnum level, const std::string& message, AdditionInfo info = {}) {
            {
                std::lock_guard lock(mtx);
                processRecord(LogRecord{level, message, info});
            }
        }

        /**
         * @brief 添加日志[模板]
         * @details 他似乎不需要详细注释[划掉]
         * @tparam Level 日志等级
         * @param message 日志消息
         * @param info 日志附加信息
         */
        template<LogLevelEnum Level>
        void log(const std::string& message, AdditionInfo info = {}) {
            log(Level, message, info);
        }

    private:
        std::unique_ptr<BaseFormatter> _formatter;
        std::vector<std::unique_ptr<BaseFilter>> _filters;
        std::vector<std::unique_ptr<BaseHandler>> _handlers;

        std::mutex mtx;

        /**
         * @brief 构建者使用的日志器构造
         * @details 他似乎不需要详细注释[划掉]
         * @param filters 过滤器数组
         * @param formatter 格式化器
         * @param handlers 处理器数组
         */
        Logger(std::vector<std::unique_ptr<BaseFilter>> filters,
           std::unique_ptr<BaseFormatter> formatter,
           std::vector<std::unique_ptr<BaseHandler>> handlers):
                _filters(std::move(filters)),
                _formatter(std::move(formatter)),
                _handlers(std::move(handlers)) {}

        void processRecord(const LogRecord& record) {
            for (auto& e : _filters) {
                if (!e->isThrough(record)) {return;}
            }
            std::string str = _formatter->format(record);
            for (auto& e : _handlers) {
                e->execute(record, str);
            }
        }
};

