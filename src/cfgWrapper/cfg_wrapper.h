// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _CFG_WRAPPER_H_
#define _CFG_WRAPPER_H_

// C++ Standard Library headers
#include <ctime>
#include <iostream>
// External Library headers

// mdt-dialout-collector Library headers
#include "logs_handler.h"
#include "cfg_handler.h"


// C++ Class
class CfgWrapper {
public:
    CfgWrapper() {
        spdlog::get("multi-logger")->
            debug("constructor: CfgWrapper()"); };
    ~CfgWrapper() {
        spdlog::get("multi-logger")->
            debug("destructor: ~CfgWrapper()"); };

    bool BuildCfgWrapper();

    void DisplayCfgWrapper();

    // Setters
    void set_writer_id() {
        this->writer_id = main_cfg_parameters.at("writer_id");
    };

    // Getters
    std::string &get_writer_id() { return this->writer_id; };
private:
    std::string writer_id = "mdt-dialout-collector";
};

#endif

