// Strategy declaration

#pragma once
#include <string>
#include <chrono>
#include <mutex>
#include <deque>
#include <unordered_map>

class RestAPI;
class Logger;

struct OrderInfo {
    std::string order_id;
    std::string side;
    double price;
    double amount;
    std::chrono::steady_clock::time_point timestamp;
};

struct FillInfo {
    std::string order_id;
    std::string side;
    double amount;
};

class Strategy {
private:
    static constexpr double TICK_SIZE = 0.0001; // mimnum price change unit
    static constexpr double BASE_TRADE_SIZE = 100.0; // change by Volatility, Inventory..
    static constexpr double STALE_TIMEOUT_MS = 5000; 

    std::mutex mtx_;
    RestAPI* api_;
    Logger* logger_;
    // volatility
    std::deque<double> price_window;
    // protfolio
    std::unordered_map<std::string, OrderInfo> active_orders;
    double inventory = 0.0;

    double getVolatility();
    double getDynamicTradeSize();

public:
    Strategy(Logger* logger);

    void simpleStrategy(double bid, double ask);
    void updateInventory(const FillInfo& fill);
};