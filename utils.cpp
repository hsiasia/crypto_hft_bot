// Tool functions implementation (e.g. HMAC, Signed)

#include "config.h"
#include "utils.h"

#include <sstream>
#include <vector>
#include <openssl/hmac.h>
#include <ctime>
#include <curl/curl.h>

std::string to_hex_string(const std::vector<unsigned char>& data) {
    // as a string buffer
    std::ostringstream oss;
    // fill char with '0'
    oss.fill('0');
    // output as hexadecimal format
    oss << std::hex;
    for (unsigned char byte: data) {
        // set the width as 2(for hexadecimal format)
        oss.width(2);
        // transform output byte(char) into integar type
        oss << static_cast<int>(byte);
    }

    return oss.str();
}

std::string hmac_sha256_hex(const std::string& key, const std::string& msg) {
    unsigned int len = 0;
    // EVP_MAX_MD_SIZE is the paramater of OpenSSL, represent the max digest length
    std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);

    // reinterpret_cast<const unsigned char*> const char* -> const unsigned char*
    // static_cast<int> size_t -> int
    // str.data() return the pointer of string (with \0)
    // save the encrypted data into digest, and its length into len
    unsigned char* result = HMAC(EVP_sha256(),
                            reinterpret_cast<const unsigned char*>(key.data()), static_cast<int>(key.size()),
                            reinterpret_cast<const unsigned char*>(msg.data()), msg.size(),
                            digest.data(), &len);
    digest.resize(len);

    return to_hex_string(digest);
}

// userdata is a void* pointer point to &response in signedRequest
size_t curlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    // auto& response create alias
    auto& response = *static_cast<std::string*>(userdata);
    // ptr is the data address of from curl, which length is size * nmemb(bytes)
    response.append(ptr, size * nmemb);
    // return the length, to make sure the functions address all data
    return size * nmemb;
}

// for TRADE & USER_DATA APIs
std::string signedRequest(const std::string& method, const std::string& base_url, const std::string& data) {
    std::string header = "X-MBX-APIKEY: " + API_KEY;

    // get current time(second) and transform into millisecond(required)
    long long timestamp = static_cast<long long>(std::time(nullptr)) * 1000;
    std::string query = data + "&timestamp=" + std::to_string(timestamp);
    std::string signature = hmac_sha256_hex(SECRET_KEY, query);
    std::string full_url = base_url + "?" + query + "&signature=" + signature;

    // initialize curl session
    CURL* curl = curl_easy_init();

    // curl_slist as a linked list to save all HTTP headers
    struct curl_slist* headers = nullptr;
    // c_str: const char
    headers = curl_slist_append(headers, header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }
    else {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    auto res = curl_easy_perform(curl);
    // release resources
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

std::string toStringWithPrecision(double value, int n) {
    // stream object for transfering variable into string 
    std::ostringstream oss;
    // keep n places
    oss.precision(n);
    // std::fixed controls precision() to keep n decimal places and not by scientific notation, the precision() takes all number into account without it
    // << saves variable into stream
    oss << std::fixed << value;

    // stream to string
    return oss.str();
}