#ifndef BOOK_H
#define BOOK_H

#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>


enum struct orderType {BUY, SELL};

inline std::string_view toString(orderType o){
    switch(o){
        case orderType::BUY : return "BUY";
        case orderType::SELL : return "SELL";
    }
    return "UNKNOWN";
}

class order{
public:
    order(int orderId_, int price_, int volume_, orderType oType_) : orderId(orderId_), price(price_), volume(volume_), oType(oType_) {}
    int getOrderId() const{
        return orderId;
    }

    int getPrice() const{
        return price;
    }

    int getVolume() const{
        return volume;
    }

    enum orderType getType() const{
        return oType;
    }

    void reduceVolume(int vol){
        volume = volume - vol;
    }

    inline operator std::string() const {
        return "Id: " + std::to_string(orderId) + " | Price: " + std::to_string(price) + " | Volume: " + std::to_string(volume) + "\n";
    }

    std::string toJson(){
        return std::string {"{\n"} +
        "\"type\": \""+ std::string(toString(oType)) + "\", \n" +
        "  \"orderId\": " + std::to_string(orderId) + ",\n" +
        "  \"price:\": "  + std::to_string(price) + ",\n" + 
        "  \"volume:\": " + std::to_string(volume) +  "\n" +
        "}";
    }

    private:
    int orderId;
    int price;
    int volume;
    enum orderType oType;
};

class orderBook{
    public:
    orderBook(int minPrice_, int maxPrice_, int tickSize_,std::atomic<bool>& running);
    ~orderBook();

    void insertIntoBook(int orderId, orderType orderType, int price, int volume);
    void addOrder(orderType orderType, int price, int volume);
    void matchBuyOrder(int price, int& volume);
    void matchSellOrder(int price, int& volume);
    void cancelOrder(int orderId);
    bool validPrice(int price);
    std::string displayData();
    std::string toJson() const;
    void addWebOrder(nlohmann::json& orderData);
    void changeTimer();
    bool getRecentChange();

    private:
    std::vector<std::list<order>> bids;
    std::vector<std::list<order>> asks;
    int tickSize;
    int lowestPriceInd, highestBidInd;
    int minPrice, maxPrice , priceLevels;
    std::unordered_map<int, std::pair<int, std::list<order>::iterator >> orderMap; //id - > index , iterator
    int counter; //order ids 
    std::atomic<bool> recentChange;
    std::thread timerThread;
    std::atomic<bool>& running;
    mutable std::mutex bookMutex;

    void updateHighest();
    void updateLowest();
    int priceToIndex(int price);
    int indexToPrice(int index);
};

#endif