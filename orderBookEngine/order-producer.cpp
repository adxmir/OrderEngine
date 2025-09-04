#include "order-producer.hpp"

orderProducer::orderProducer(const std::string& brokers, const std::string& topic)
    {
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", brokers, errstr);
    producer = RdKafka::Producer::create(conf, errstr);
    if(!producer){
        std::cerr << "Failed to create producer \n";
    }

    delete conf;

    topicObj = RdKafka::Topic::create(producer, topic, nullptr, errstr);
    if(!topicObj){
       std::cerr << "Failed to create topic \n";
    }
}   

void orderProducer::produce(const std::string& message){
    RdKafka::ErrorCode response = producer->produce(topicObj,RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY, const_cast<char*>(message.c_str()), message.size(),nullptr, nullptr);
    if(response != RdKafka::ErrorCode::ERR_NO_ERROR){
        std::cerr << "Failed to produce message \n";
    }
}