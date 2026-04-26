// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _LOGS_HANDLER_H_
#define _LOGS_HANDLER_H_

#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


class LogsHandler {
public:
    LogsHandler();
    // Null-safe: multi-logger may not be registered yet if the post-cfg
    // sinks were never swapped in (e.g. cfg parse failed). Fall back to
    // multi-logger-boot which is registered by the constructor.
    ~LogsHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~LogsHandler()");
        } else if (auto b = spdlog::get("multi-logger-boot")) {
            b->debug("destructor: ~LogsHandler()");
        }
    };
    bool set_boot_spdlog_sinks();
    bool set_spdlog_sinks();
private:
    std::shared_ptr<spdlog::logger> multi_logger_boot;
    std::shared_ptr<spdlog::logger> multi_logger;
};

#endif

