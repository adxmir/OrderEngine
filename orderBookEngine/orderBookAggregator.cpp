#include "orderBookAggregator.hpp"

std::string Aggregator::aggregateBook(const orderBook& book) const{
    std::vector<std::pair<int,int>> readyBids = aggregateOrders(book.getBids(), orderType::BUY);
    std::vector<std::pair<int,int>> readyAsks = aggregateOrders(book.getAsks(), orderType::SELL);
     nlohmann::json j;
    j["type"] = "OrderBook Snapshot";
    j["bids"] = nlohmann::json::array();
    for(auto& p : readyBids){
        j["bids"].push_back({{"price", p.first} , {"volume" , p.second}});
    }
    j["asks"] = nlohmann::json::array();
    for(auto& p : readyAsks){
        j["asks"].push_back({{"price", p.first} , {"volume" , p.second}});
    }
    j["snapshot_time"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::cout << "j is dumped \n";
    return j.dump();
}

std::vector<std::pair<int,int>> Aggregator::aggregateOrders(const std::vector<std::list<order>>& orders, orderType type)const {
    std::vector<std::pair<int,int>> readyOrders;
    if(type == orderType::BUY){
        for(int i = orders.size() - 1; i >= 0; i--){
        if(orders[i].empty()){continue;}
        int price = orders[i].front().getPrice();
        std::pair<int,int> curr(price, 0);
        for(auto& o : orders[i]){
            curr.second += o.getVolume();
            }
        readyOrders.emplace_back(curr);
        }
    }
    else{
        for(int i = 0; i < orders.size(); i++){
        if(orders[i].empty()){continue;}
        int price = orders[i].front().getPrice();
        std::pair<int,int> curr(price, 0);
        for(auto& o : orders[i]){
            curr.second += o.getVolume();
            }
        readyOrders.emplace_back(curr);
        } 
    }
    return readyOrders;
}