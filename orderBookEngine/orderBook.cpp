#include "orderBook.hpp"

orderBook::orderBook(int minPrice_, int maxPrice_, int tickSize_, std::atomic<bool>& running_) :
    minPrice(minPrice_),
    maxPrice(maxPrice_), 
    tickSize(tickSize_),
    recentChange(false),
    running(running_)
{
    priceLevels = ( maxPrice - minPrice ) / tickSize + 1;
    bids.resize(priceLevels);
    asks.resize(priceLevels);
    lowestPriceInd = -1;
    highestBidInd = -1;
    counter = 0;
    timerThread = std::thread(&orderBook::changeTimer, this);
}

void orderBook::insertIntoBook(int orderId, orderType orderType, int price, int volume){
    std::vector<std::list<order>>* currBook;
    int index = priceToIndex(price);
    if(orderType == orderType::BUY){
        currBook = &bids;
        if(index > highestBidInd) highestBidInd = index;
    }
    else{
        currBook = &asks;
        if(lowestPriceInd == -1 || index < lowestPriceInd) lowestPriceInd = index;
    }
    (*currBook)[index].push_back(order(counter, price, volume, orderType));
    auto it = std::prev((*currBook)[index].end());
            orderMap[counter] = {index,it};

}
    
    
void orderBook::addOrder(orderType orderType, int price, int volume){
    counter++;
    recentChange = true;
    if(orderType == orderType::BUY){
        matchBuyOrder(price, volume);
        if(volume > 0){
            insertIntoBook(counter, orderType::BUY, price, volume);
        }
    }
    else{
        matchSellOrder(price, volume);
        if(volume > 0){
            insertIntoBook(counter, orderType::SELL, price, volume);
        }
    }
}

void orderBook::matchBuyOrder(int price, int& volume){
    while(lowestPriceInd != -1 && indexToPrice(lowestPriceInd) <= price && volume > 0){
        auto &list = asks[lowestPriceInd];
        auto it = list.begin();
        while(it != list.end() && volume > 0){
            int amount = std::min(volume, it->getVolume());
            volume -= amount;
            if(amount == it->getVolume()){
                orderMap.erase(it->getOrderId());
                it = list.erase(it);
            }
            else{
                it->reduceVolume(amount);
                it++;
            }
        }   
        if (list.empty()) {
                updateLowest(); 
            if(lowestPriceInd == -1) break;
        }
    }

}

void orderBook::matchSellOrder(int price, int& volume){
    while(highestBidInd != -1 && indexToPrice(highestBidInd) >= price && volume > 0){
        auto &list = bids[highestBidInd];
        auto it = list.begin();
        while(it != list.end() && volume > 0){
            int amount = std::min(volume, it->getVolume());
            volume -= amount;
            if(amount == it->getVolume()){
                orderMap.erase(it->getOrderId());
                it = list.erase(it);
            }
            else{
                it->reduceVolume(amount);
                it++;
            }
        }   
        if (list.empty()) {
                updateHighest(); 
                if(highestBidInd == -1) break;
        }
    }
}

void orderBook::cancelOrder(int orderId){
    auto cancelledOrder = orderMap.find(orderId);
    if(cancelledOrder == orderMap.end()){
        return;
    }
    auto &[index, itr] = cancelledOrder->second;
    std::vector<std::list<order>>* currBook = (itr->getType() == orderType::BUY) ? &bids : &asks;
    orderType t = itr->getType();
    (*currBook)[index].erase(itr);
    if(t== orderType::BUY && highestBidInd == index && (*currBook)[index].empty()){
        updateHighest();
    }
    else if(t== orderType::SELL && lowestPriceInd == index && (*currBook)[index].empty()){
        updateLowest();
    }
    orderMap.erase(cancelledOrder);
}

bool orderBook::validPrice(int price){
        return price >= minPrice && price <= maxPrice ? true : false;
    }

void orderBook::addWebOrder(nlohmann::json& orderData){
    try{
        int price = orderData["price"];
        if(price < minPrice || price > maxPrice){
            return;
        }
        std::string type = orderData["type"];
        if(type != "BUY" && type != "SELL"){
            return;
        }
        int volume = orderData["volume"];
        if(!volume > 0){
            return;
        }
        orderType t = type == "BUY" ? orderType::BUY : orderType::SELL;
        addOrder(t, price, volume);
    }
    catch(std::exception& e){
       std::cerr << "Error occured: " << e.what() << "\n";;
    }
}

std::string orderBook::toJson() const {
    std::lock_guard<std::mutex> lock(bookMutex);
    std::cout << " succesful to json ran \n";
    return (*bookAggregator).aggregateBook(*this);

}   

void orderBook::changeTimer(){
    while(running){
        if(recentChange){
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            recentChange = false;
        }
        else{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool orderBook::getRecentChange(){
    return recentChange;
}

void orderBook::updateHighest(){
    for(int i = highestBidInd; i >= 0; i--){
        if(!bids[i].empty()){
            highestBidInd = i;
            return;
        }
    }
    highestBidInd = -1;
}

void orderBook::updateLowest(){
    for(int i = lowestPriceInd; i < priceLevels; i++){
        if(!asks[i].empty()){
            lowestPriceInd = i; 
            return;
        }
    }
    lowestPriceInd = -1;
}

orderBook::~orderBook(){
    if(timerThread.joinable()){
        timerThread.join();
    }
}