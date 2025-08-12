
#include "config.h"
#include "rest_api.h"
#include "logger.h"
#include "strategy.h"
#include "ws_client.h"

#include <iostream>

int main() {
    std::cout << "Start transaction...\n";

    Logger logger;
    Strategy strategy(&logger);
    WSClient ws(&strategy);
    ws.start(SYMBOL);

    return 0;
}
