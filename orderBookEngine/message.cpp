#include "message.hpp"

messageParser::messageParser() {
    type = "";
    price = -1;
    volume = -1;
}

void messageParser::evaluate(const std::string& message){
    std::istringstream iss(message);
    std::string command;
    int price_, volume_;
    iss >> command >> price_ >> volume_;
    type = command;
    price = price_;
    volume = volume_;
}
    
bool messageParser::validateFormat(const std::string& message){
    std::istringstream iss(message);
    std::string command;
    int price, volume ;
    if(!(iss >> command >> price >> volume)){
        return false;
    }
    if(command != "BUY" && command != "SELL"){
        return false;
    }

    if(price <= 0 || volume <= 0){
        return false;
    }
    return true;

}

int messageParser::getPrice(){
    return price;
}

int messageParser::getVolume(){
    return volume;
}

std::string messageParser::getType(){
    return type;
}

