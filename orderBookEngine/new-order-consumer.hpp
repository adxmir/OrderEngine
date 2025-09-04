#ifndef NEWORDER_H
#define NEWORDER_H

#include <iostream>
#include <string>
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>
#include "orderBook.hpp"
#include <mutex>

class orderConsumer{
    public:
    orderConsumer(const std::string& brokers, const std::string& topic_, orderBook& book_);
    ~orderConsumer();
    void consume(std::atomic<bool>& running);

    private:
    RdKafka::KafkaConsumer* consumer;
    const std::string topic;
    orderBook& book;
    std::mutex bookMutex;
    

};

#endif