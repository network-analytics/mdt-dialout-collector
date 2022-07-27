// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT


#ifndef _DATA_DELIVERY_H_
#define _DATA_DELIVERY_H_

// External Library headers
#include "kafka/KafkaProducer.h"
// mdt-dialout-collector Library headers
#include "cfg_handler.h"


class DataDelivery {
public:
    // Handling data delivery to KAFKA
    bool AsyncKafkaProducer(const std::string &json_str);
};

#endif

