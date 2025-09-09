#include <iostream>
#include "Servers/server.hpp"
#include "message.hpp"
#include "new-order-consumer.hpp"
#include "timer.hpp"

#include <signal.h>

using namespace std;


int main() {   
    Latency::timer t;
    std::string host = "0.0.0.0";
    int port = 9000;
    std::atomic<bool> running{true};
    orderBook orderEngine(100, 1500 , 1,running); // min price, max price , tick
    orderProducer oProducer("localhost:9094", "orders");
    orderConsumer oConsumer("localhost:9094", "incoming-orders", orderEngine);
    Server server(host, port, orderEngine, oProducer);
    std::thread consumerThread([&](){
        oConsumer.consume(running);
    });

    try{
        cout << "Server starting \n";
        server.startServer();
        std::string command;
        while (std::cin >> command, command != "quit") {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "'quit' command entered. Stopping the web server.. \n";
        running = false;
        if(consumerThread.joinable()) consumerThread.join();
        server.closeServer();
    }
    catch(exception& e){
        cout << "Error : " << e.what() << "\n";
        running = false;
        if(consumerThread.joinable()) consumerThread.join();
        server.closeServer();
    }
    
    return 0;
}
