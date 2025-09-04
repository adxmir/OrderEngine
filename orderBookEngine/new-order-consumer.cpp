#include "new-order-consumer.hpp"


orderConsumer::orderConsumer(const std::string& brokers, const std::string& topic_, orderBook& book_):
topic(topic_),
book(book_)
{
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", brokers,errstr);
    conf->set("group.id", "new-order-consumers", errstr);
    consumer = RdKafka::KafkaConsumer::create(conf,errstr);
    if(!consumer){
        std::cerr << "Failed to create consumer \n";;
    }
    RdKafka::ErrorCode resp = consumer->subscribe({topic});
    if(resp != RdKafka::ErrorCode::ERR_NO_ERROR){
        std::cerr << "Failed to subscribe \n";;
    }
    delete conf;

}

void orderConsumer::consume(std::atomic<bool>& running){
    try{
        while(running){
            RdKafka::Message* message = consumer->consume(1000);
            switch(message->err()){
                case RdKafka::ERR_NO_ERROR:{
                    std::string data = static_cast<const char*>(message->payload());
                    nlohmann::json orderData = nlohmann::json::parse(data);
                    {
                        std::lock_guard<std::mutex> lock(bookMutex);
                        book.addWebOrder(orderData);
                    }
                    break;
                }
                case RdKafka::ERR__TIMED_OUT:
                    break;
                default:
                    std::cerr << "Consumer error: " << message->errstr() << "\n";
                    break;
            }
            delete message;
        }
        delete consumer;
    }
    catch(std::exception& e){
        std::cerr << "Error occured: " << e.what() << "\n";
    }
}

orderConsumer::~orderConsumer(){
    if (consumer) {
        consumer->close();
    }
    RdKafka::wait_destroyed(5000);
}