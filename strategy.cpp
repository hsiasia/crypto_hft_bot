// Strategy implementation

#include "config.h"
#include "strategy.h"
#include "rest_api.h"
#include "logger.h"

#include <string>
#include <cmath>
#include <chrono> // timestamp
#include <mutex>

Strategy::Strategy(Logger* logger): logger_(logger) {}

double Strategy::getVolatility() {
    int price_size = price_window.size();
    if (price_size < 2) {
        return 0.0;
    }

    // mean
    double sum = 0.0;
    for (auto price : price_window) {
        sum += price;
    }
    double mean = sum / price_size;

    // standard deviation
    double diff;
    double square_diff_sum = 0.0;
    for (auto price : price_window) {
        diff = price - mean;
        square_diff_sum += diff * diff;
    }

    // variance
    double variance = square_diff_sum / (price_size - 1);
    // square root
    double volatility = std::sqrt(variance);

    return volatility;
}

double Strategy::getDynamicTradeSize() {
    double volatility = getVolatility();

    // volatility higher, size lower
    double size = BASE_TRADE_SIZE * (0.5 / volatility);

    // control size base on inventory, when inventory in a high rate, reduce the current size
    double inventory_factor = std::max(0.1, 1.0 - std::abs(inventory) / (BASE_TRADE_SIZE * 10));

    size *= inventory_factor;

    // lowerbound, upperbound trade size
    if (size < 10) {
        size = 10;
    }
    if (size > 500) {
        size = 500;
    }

    return size;
}

// Somebody want to buy on bid price, sell on ask price. 
void Strategy::simpleStrategy(double bid, double ask) {
    // static: initialize one time, kind of global variable but only in this function. 
    // static RestAPI api;
    // get current time
    auto now = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(mtx_);

        // Cancel stale order
        for (auto it = active_orders.begin(); it != active_orders.end(); ) {
            // .count() transform milliseconds into integar
            auto order_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - (it -> second.timestamp)).count();
            
            if (order_ms > STALE_TIMEOUT_MS) {
                // api_ ->deleteOrder(it -> second.order_id);
                it = active_orders.erase(it);
                logger_ -> log("Cancel " + it -> second.side + 
                    " price: " + std::to_string(it -> second.price) + 
                    " amount: " + std::to_string(it -> second.amount)
                );
            } else {
                it++;
            }
        }

        // limit trading base on inventory
        bool allow_buy = inventory < BASE_TRADE_SIZE * 5;
        bool allow_sell = inventory > -BASE_TRADE_SIZE * 5;

        // dynamic trade size
        double trade_size = getDynamicTradeSize();

        if (allow_buy) {
            // TODO: update TICK_SIZE times by volatility
            double buy_price = bid - TICK_SIZE;
            std::string buy_order_id = api_ ->postOrder("BUY", trade_size, buy_price);
            active_orders[buy_order_id] = {buy_order_id, "BUY", buy_price, trade_size, now};
            logger_ -> log("Place BUY order: price=" + std::to_string(buy_price) + 
                " size=" + std::to_string(trade_size)
            );
        }

        if (allow_sell) {
            // TODO: update TICK_SIZE times by volatility
            double sell_price = ask + TICK_SIZE;
            std::string sell_order_id = api_ ->postOrder("SELL", trade_size, sell_price);
            active_orders[sell_order_id] = {sell_order_id, "SELL", sell_price, trade_size, now};
            logger_  -> log("Place SELL order: price=" + std::to_string(sell_price) + 
                " size=" + std::to_string(trade_size)
            );
        }
    }
}

void Strategy::updateInventory(const FillInfo& fill) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = active_orders.find(fill.order_id);
    if (it != active_orders.end()) {
        if (fill.side == "BUY") {
            inventory += fill.amount;
        } else if (fill.side == "SELL") {
            inventory -= fill.amount;
        }
        active_orders.erase(it);

        logger_ -> log("Fill " + fill.side + 
            " amount: " + std::to_string(fill.amount) + 
            ", current inventory: " + std::to_string(inventory)
        );
    }
}