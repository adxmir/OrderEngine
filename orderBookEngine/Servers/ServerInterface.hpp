#ifndef SERVERIF_H
#define SERVERIF_H

#include <iostream>
#include <mutex>
#include <atomic>
#include "orderBook.hpp"
#include <thread>
#include <chrono>
#include "message.hpp"
#include "order-producer.hpp"

class serverInterface{
    public:
    virtual void startServer() = 0;
    virtual void closeServer() = 0;
    virtual ~serverInterface() = default;  

    //virtual void messageClient();
};

#endif