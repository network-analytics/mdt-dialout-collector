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


// Centralizing logging parameters
extern std::shared_ptr<spdlog::logger> multi_logger;

class LogsHandler {
public:
    LogsHandler();
    bool set_spdlog_sinks(std::vector<spdlog::sink_ptr> &spdlog_sinks,
        std::shared_ptr<spdlog::logger> &multi_logger);
protected:
    std::vector<spdlog::sink_ptr> spdlog_sinks;
    std::shared_ptr<spdlog::logger> multi_logger;
};

#endif

