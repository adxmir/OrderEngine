#include "serverBoost.hpp"

serverBoost::serverBoost(const std::string& host_, int port_, orderBook& engine):
    book(engine),
    port(port_),
    host(host_),
    io(),
    acceptor(io),
    clientsN(0)
    {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port));
        acceptor.open(ep.protocol());
        acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.bind(ep);
    }


void serverBoost::startServer(){
    acceptor.listen();
    listen();
    for(int i = 0; i < threadSize;i++){
        workerThreads[i] = std::make_shared<std::thread>(&serverBoost::handleEvent,this);
    }
}

void serverBoost::closeServer(){
    io.stop();
    for(auto& t : workerThreads) {
        if(t && t->joinable()) t->join();
    }
}

void serverBoost::listen(){
    auto sock = std::make_shared<boost::asio::ip::tcp::socket>(io);
    acceptor.async_accept(*sock, [this, sock](boost::system::error_code e){
        if(!e){
            readMessage(sock);
        }
        listen();
    });
}

void serverBoost::readMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket){
    auto buffer = std::make_shared<std::array<char,1024>>();
    socket -> async_read_some(boost::asio::buffer(*buffer), [this,socket, buffer](const boost::system::error_code& error, size_t length){
        if(!error){
            const std::string message(buffer->data(), length);
            std::string response = processMessage(message);
            boost::asio::async_write(*socket , boost::asio::buffer(response),[socket](const boost::system::error_code& e, std::size_t ){});
            readMessage(socket);
        }
    });
}

std::string serverBoost::processMessage(const std::string& message){
    messageParser parser;
    if(!parser.validateFormat(message)){
        return "Invalid format \n";
    }
    parser.evaluate(message);
    orderType t;
    if(parser.getType() == "BUY"){
        t = orderType::BUY;
    }
    else{
        t = orderType::SELL;
    }
    if(!book.validPrice(parser.getPrice())){
        return "Invalid price \n";
    }
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::lock_guard<std::mutex> lock(bookMutex);
        book.addOrder(t, parser.getPrice(), parser.getVolume());
        auto end = std::chrono::high_resolution_clock::now();
        auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Latency : " << latency_us << "\n";
    }
    std::string response = "Order fufilled \n";
    return response;
}

void serverBoost::handleEvent(){
    try{
        io.run();
    }
    catch(std::exception &e){
        std::cerr << "Error occured in worker threads : " << e.what() << std::endl;
    }
}