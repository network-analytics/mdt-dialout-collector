// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _LOGS_HANDLER_H_
#define _LOGS_HANDLER_H_

// C++ Standard Library headers
#include <iostream>
#include <vector>
// External Library headers
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


class LogsHandler {
public:
    LogsHandler();
    ~LogsHandler() {
        spdlog::get("multi-logger")->debug("destructor: ~LogsHandler()"); };
    bool set_boot_spdlog_sinks();
    bool set_spdlog_sinks();
private:
    std::shared_ptr<spdlog::logger> multi_logger_boot;
    std::shared_ptr<spdlog::logger> multi_logger;
};

#endif

