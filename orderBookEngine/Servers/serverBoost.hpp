#ifndef SERVERB_H
#define SERVERB_H

#include "ServerInterface.hpp"
#include <boost/asio.hpp>

class serverBoost : public serverInterface{
    public:
    serverBoost(const std::string& host, int port, orderBook& engine);

    void startServer();
    void closeServer();

    private:

    static constexpr int threadSize = 5;
    const std::string& host;
    int port;

    orderBook& book;
    std::shared_ptr<std::thread> workerThreads[threadSize];
    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor acceptor;
    std::mutex bookMutex;
    int clientsN;

    void handleEvent();
    void listen();
    void readMessage(std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    std::string processMessage(const std::string&);

};

#endif