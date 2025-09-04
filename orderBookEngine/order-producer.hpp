#ifndef ORDERP_H
#define ORDERP_H

#include <iostream>
#include <string>
#include <librdkafka/rdkafkacpp.h>

class orderProducer{
    public:
    orderProducer(const std::string& brokers, const std::string& topic);
    
    void produce(const std::string& message);

    ~orderProducer(){
        producer->flush(1000);
        delete producer;
        delete topicObj;
    }

    private:
    RdKafka::Producer* producer;
    RdKafka::Topic* topicObj;
};

#endif