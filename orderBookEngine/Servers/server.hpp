#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <sys/socket.h>      
#include <netinet/in.h>     
#include <arpa/inet.h>      
#include <unistd.h>      
#include <string.h>         
#include <errno.h>  
#include <cstdint>
#include <stdio.h>
#include <fcntl.h>
#include <sys/event.h> 
#include "ServerInterface.hpp"



class Server : public serverInterface{
    public:
        Server(const std::string& host, int port, orderBook& engine, orderProducer& orderProducer);

        std::atomic<bool> running;
        
        void startServer() override;
        void closeServer() override;

    private:
        static constexpr int threadSize = 5;
        static constexpr int maxSize = 64;
        orderBook& book;
        orderProducer& kafkaProducer;
        std::string host;
        int port;
        int socketFD;
        struct kevent events[threadSize][maxSize];
        int workerQueues[threadSize];
        std::thread listener;
        std::thread clientThreads[threadSize];
        std::vector<int> clients;
        std::mutex clientsMutex;
        std::thread broadcaster;
        std::mutex bookMutex;

            void createSocket();
            void Listen();
            void setUpKqueue();
            void handleEvent(int clientN);
            std::string processEvent(const std::string& message);
            void fillKQueue(int worker, int fd);
            void setNonBlocking(int fd);
            void broadcast();
    };

#endif