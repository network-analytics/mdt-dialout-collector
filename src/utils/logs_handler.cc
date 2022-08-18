// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "logs_handler.h"
#include "cfg_handler.h"


// Centralizing logging parameters
std::unique_ptr<LogsHandler>
    logs_handler(new LogsHandler());

std::shared_ptr<spdlog::logger> multi_logger =
    spdlog::get("multi-logger");

LogsHandler::LogsHandler()
{
    if (set_spdlog_sinks(
            this->spdlog_sinks,
            this->multi_logger,
            this->spdlog_level) == false) {
        std::cout << "Unable to LogsHandler::set_spdlog_sinks(...)\n";
        std::exit(EXIT_FAILURE);
    } else {
        multi_logger->debug("constructor: LogsHandler()");
    }
}

bool LogsHandler::set_spdlog_sinks(
    std::vector<spdlog::sink_ptr> &spdlog_sinks,
    std::shared_ptr<spdlog::logger> &multi_logger,
    std::string &spdlog_level)
{
    spdlog_level = logs_cfg_parameters.at("spdlog_level");

    // Syslog
    if (logs_cfg_parameters.at("syslog").compare("true") == 0) {
        const std::string ident = "mdt-dialout-collector";
        try {
            auto spdlog_syslog =
                std::make_shared<spdlog::sinks::syslog_sink_mt>(
                    ident, 0, LOG_LOCAL3, true);
            spdlog_sinks.push_back(spdlog_syslog);
        } catch (const spdlog::spdlog_ex &sex) {
            std::cout << "spdlog, syslog: " << sex.what() << "\n";
            return false;
        }
    }

    // ConsoleLog
    if (logs_cfg_parameters.at("console_log").compare("true") == 0){
        try {
            auto spdlog_console =
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            spdlog_sinks.push_back(spdlog_console);
        } catch (const spdlog::spdlog_ex &sex) {
            std::cout << "spdlog, console: " << sex.what() << "\n";
            return false;
        }
    }

    multi_logger = std::make_shared<spdlog::logger>
        ("multi-logger", begin(spdlog_sinks), end(spdlog_sinks));
    multi_logger->set_level(spdlog::level::from_str(spdlog_level));
    spdlog::register_logger(multi_logger);

    return true;
}

