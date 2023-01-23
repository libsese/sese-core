/**
 * @file LogHelper.h
 * @author kaoru
 * @date 2022年4月17日
 * @brief 日志工具类
 */
#pragma once
#include <sese/record/Logger.h>
#include "Logger.h"
#include "Event.h"

namespace sese::record {
    /**
     * @brief 日志工具
     */
    class API LogHelper {
    public:
        using Ptr = std::shared_ptr<LogHelper>;

        /**
         * 初始化日志工具
         * @param tag 指定标签名称
         */
        explicit LogHelper(const char *tag = RECORD_DEFAULT_TAG);

        /// 输出 DEBUG 级别的日志
        void debug(const char *format, ...);
        /// 输出 INFO 级别的日志
        void info(const char *format, ...);
        /// 输出 WARN 级别的日志
        void warn(const char *format, ...);
        /// 输出 ERROR 级别的日志
        void error(const char *format, ...);

        /// 输出 DEBUG 级别的日志
        /// @deprecated 已知会和已有函数冲突
        // void debug(const char *file, int32_t line, const char *format, ...);
        /// 输出 INFO 级别的日志
        /// @deprecated 已知会和已有函数冲突
        // void info(const char *file, int32_t line, const char *format, ...);
        /// 输出 WARN 级别的日志
        /// @deprecated 已知会和已有函数冲突
        // void warn(const char *file, int32_t line, const char *format, ...);
        /// 输出 ERROR 级别的日志
        /// @deprecated 已知会和已有函数冲突
        // void error(const char *file, int32_t line, const char *format, ...);

    private:
        void log(Level level, const char *file, int32_t line, const char *format, va_list ap);

        const char *filter = nullptr;
        const Logger *logger = nullptr;
    };
}// namespace sese::record