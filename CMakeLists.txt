cmake_minimum_required(VERSION 3.10)
project(usdc_hft_bot)

set(CMAKE_CXX_STANDARD 17)

# Find the path, lib link on your server. 
# All of these package need to be install by brew on mac os. 
# TODO: We could use cvpkg to help us (TODO). 
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(usdc_hft_bot
    main.cpp
    logger.cpp
    rest_api.cpp
    strategy.cpp
    utils.cpp
    ws_client.cpp
)

# Where to find .h / .hpp, so that we can use #include <...> 或 #include "..."
# include_directories() is kind of global setting, not recommend
# target_include_directories() is for particular target, recommend, cleaness
target_include_directories(usdc_hft_bot PRIVATE # "PRIVATE, INTERFACE, PUBLIC" to control inheritable parameters
    ${CURL_INCLUDE_DIRS} # variable from find_package
    ${OPENSSL_INCLUDE_DIR} # variable from find_package
    ${Boost_INCLUDE_DIRS} # variable from find_package
    # from path (for websocketpp), WebSocket++ is header-only library. (deprecated, linux c++ compiler defaultly scaning /usr/include)
)

# Which library (.lib、.a、.so) need to be linked
# Modern cake will include path automatically. 
target_link_libraries(usdc_hft_bot
    ${CURL_LIBRARIES}
    OpenSSL::SSL # modern target, excepted sitiuation
    OpenSSL::Crypto # modern target, excepted sitiuation
    ${Boost_LIBRARIES}
    nlohmann_json::nlohmann_json # modern target
)