// REST API implementation

#include "config.h"
#include "rest_api.h"
#include "utils.h"

#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

RestAPI::RestAPI() {}

// GET Account info
std::string RestAPI::getAccountInfo() {
    std::string url = BASE_URL + "/api/v3/account";
    return signedRequest("GET", url, "");
}

std::string RestAPI::getListenKey() {
    std::string header = "X-MBX-APIKEY: " + API_KEY;
    std::string url = BASE_URL + "/api/v3/userDataStream";

    CURL* curl = curl_easy_init();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    auto j = json::parse(response);
    return j["listenKey"].get<std::string>();
}

// GET All Order
// std::string RestAPI::getAllOrder();

// GET Order
// std::string RestAPI::getOrderByID(const std::string& orderID);

// POST Order
std::string RestAPI::postOrder(const std::string& side, double quantity, double price) {
    std::string url = BASE_URL + "/api/v3/order";
    std::string data = "symbol=" + SYMBOL + "&side=" + side +
        "&type=LIMIT&timeInForce=GTC&quantity=" + std::to_string(quantity) +
        "&price=" + toStringWithPrecision(price, 6);
    return signedRequest("POST", url, data);
}

// DELETE Order
std::string RestAPI::deleteOrder(const std::string& orderId) {
    std::string url = BASE_URL + "/api/v3/order";
    std::string data = "symbol=" + SYMBOL + "&orderId=" + orderId;
    return signedRequest("DELETE", url, data);
}