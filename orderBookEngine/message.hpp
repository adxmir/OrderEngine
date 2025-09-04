#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

class messageParser{
    public:
    messageParser();
    void evaluate(const std::string& message);
    bool validateFormat(const std::string& message);

    int getPrice();
    int getVolume();
    std::string getType();

    private:
    std::string type;
    int price;
    int volume;
};

#endif