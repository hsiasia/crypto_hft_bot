// Tool functions declaration

#pragma once
#include <string>

size_t curlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
std::string signedRequest(const std::string& method, const std::string& url, const std::string& data);
std::string toStringWithPrecision(double value, int n = 5);