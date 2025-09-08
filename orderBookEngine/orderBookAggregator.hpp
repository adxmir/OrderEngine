#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <iostream>
#include "orderBook.hpp"
#include <vector>
#include <utility>

enum class orderType;
class order;         
class orderBook;       


class Aggregator{
    public:
    Aggregator();

    std::string aggregateBook(const orderBook& book) const;
    std::vector<std::pair<int,int>> aggregateOrders(const std::vector<std::list<order>>& orders, orderType type)const;
};

#endif