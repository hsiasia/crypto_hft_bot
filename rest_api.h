// REST API declaration

#pragma once
// Can ignore, only implement function or variable need it. 
#include <string>

class RestAPI {
public:
    RestAPI();
    
    std::string getListenKey();
    std::string getAccountInfo();

    // std::string getAllOrder();
    // std::string getOrderByID(const std::string& orderID);
    std::string postOrder(const std::string& side, double quantity, double price);
    std::string deleteOrder(const std::string& orderID);
};