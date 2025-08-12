// WebSocket implementation

#include "config.h"
#include "ws_client.h"
#include "rest_api.h"
#include "strategy.h"

// WebSocket++ suppout two version
// asio_no_tls / asio need Boost.Asio or standalone Asio -> async version
// none no need for Boost -> sync, limited use
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <string>

// Reference nlohmann::json as json, for futher code cleaness. 
using json = nlohmann::json;
// Alias. Give a long type name into a short one. 
// Or "asio_client client" will be websocketpp::client<websocketpp::config::asio_client> client. 
typedef websocketpp::client<websocketpp::config::asio_tls_client> asio_client; 

static std::shared_ptr<boost::asio::ssl::context> on_tls_init(websocketpp::connection_hdl) {
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);

    try {
        ctx -> set_options(boost::asio::ssl::context::default_workarounds |
                        boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 |
                        boost::asio::ssl::context::single_dh_use);

        // verify_none: close cert verification during dev, verify_peer: in prod
        ctx -> set_verify_mode(boost::asio::ssl::verify_none);
    } catch (std::exception& e) {
        std::cerr << "Error in TLS context setup: " << e.what() << std::endl;
    }

    return ctx;
}

// Setting strategy_ = strategy; as soon as initilize. It has the same result as in {}. 
// But this method is faster and standand (for varaible, reference, huge object). 
WSClient::WSClient(Strategy* strategy) : strategy_(strategy) {}

void WSClient::connectUserStream() {
    std::string listenKey = restApi_ -> getListenKey();
    
    asio_client client;
    std::string url = WS_URL + "/" + listenKey;

    client.init_asio();
    client.set_tls_init_handler(on_tls_init);

    client.set_message_handler([this](websocketpp::connection_hdl, asio_client::message_ptr msg) {
        auto message = json::parse(msg->get_payload());

        if (message.contains("e") && message["e"] == "executionReport") {
            FillInfo fill_info;
            fill_info.order_id = message["c"].get<std::string>(); // clientOrderId
            fill_info.side = message["S"].get<std::string>();
            fill_info.amount = std::stod(message["l"].get<std::string>()); // last filled qty

            std::cout << "order_id: " << fill_info.order_id << "side: " << fill_info.side << "amount: " << fill_info.amount << std::endl;
            strategy_ -> updateInventory(fill_info);
        }
    });

    websocketpp::lib::error_code errorCode;
    auto con = client.get_connection(url, errorCode);
    client.connect(con);
    client.run();
}

void WSClient::connectMarket(const std::string& symbol) {
    // Initialize WebSocket++ object. 
    asio_client client;
    std::string url = WS_URL + "/" + symbol + "@bookTicker";

    // Initialize asynchronous client based on boost asio. 
    client.init_asio();

    // 加入 TLS 初始化函式（這是啟用 wss 的關鍵）
    client.set_tls_init_handler(on_tls_init);

    // Lambda function, to address the callback when websocket receieves message. 
    // [this] means the lambda will catch the outside this pointer (WSClient object) for calling strategy_ -> onPriceUpdate(). [] could not do this. 
    // TODO: We could warp another function to replace the lambda function for readness reason. 
    client.set_message_handler([this](websocketpp::connection_hdl, asio_client::message_ptr msg) {
        // Parse payload(JSON) into json object. 
        // object. v.s. pointer->
        auto message = json::parse(msg -> get_payload());
        // Get string to double. 
        if (message.contains("b") && message.contains("a")) {
            double bid = std::stod(message["b"].get<std::string>());
            double ask = std::stod(message["a"].get<std::string>());

            std::cout << "bid: " << bid << "ask: " << ask << std::endl;
            strategy_ -> simpleStrategy(bid, ask);
        }
    });

    // Error code object. 
    websocketpp::lib::error_code errorCode;
    // Client manages connection. 
    auto con = client.get_connection(url, errorCode);
    client.connect(con);
    client.run();
}

void WSClient::start(const std::string& symbol) {
    std::thread market_thread(&WSClient::connectMarket, this, symbol);
    std::thread user_thread(&WSClient::connectUserStream, this);

    market_thread.join();
    user_thread.join();
}