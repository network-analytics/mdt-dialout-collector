#ifndef _DATA_DELIVERY_H_
#define _DATA_DELIVERY_H_

#include <iostream>


class DataDelivery {
public:
    // Handling data delivery to KAFKA
    bool async_kafka_producer(const std::string& json_str);
};

#endif

