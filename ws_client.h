// WebSocket declaration

#pragma once
#include <string>

// Forward declaration. We'll use it afterwards, but right now we don't have to know how it's implement. 
// No need to include strategy.h right now, to prevent from include dependency. Faster. 
// Could only be use by pointer or reference type, e.g. Strategy* strategy_;. 
class RestAPI;
class Strategy;

class WSClient {
private:
    RestAPI* restApi_;
    Strategy* strategy_;

    void connectUserStream();
    void connectMarket(const std::string& symbol);
    
public:
    WSClient(Strategy* strategy);
    void start(const std::string& symbol);
};