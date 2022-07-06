#ifndef _DATA_DELIVERY_H_
#define _DATA_DELIVERY_H_

#include <iostream>


class DataDelivery {
public:
    // Handling data delivery to KAFKA
    int async_kafka_producer(const std::string& json_str);
};

#endif

