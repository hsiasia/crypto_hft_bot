// Logger implementation

#include "logger.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <iostream>

class Logger::Impl {
public:
    std::queue<std::string> log_queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::thread worker;

    Impl() {
        worker = std::thread([this]{
            while (running) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]{ return !log_queue.empty() || !running; });

                while (!log_queue.empty()) {
                    std::string msg = log_queue.front();
                    log_queue.pop();
                    lock.unlock();

                    std::cout << "[LOG] " << msg << std::endl;

                    lock.lock();
                }
            }
            // rest msg
            while (!log_queue.empty()) {
                std::cout << "[LOG] " << log_queue.front() << std::endl;
                log_queue.pop();
            }
        });
    }

    ~Impl() {
        running = false;
        cv.notify_all();
        if (worker.joinable()) worker.join();
    }
};

Logger::Logger() {
    pImpl = new Impl();
}

Logger::~Logger() {
    delete pImpl;
}

void Logger::log(const std::string& msg) {
    {
        std::lock_guard<std::mutex> lock(pImpl->mtx);
        pImpl->log_queue.push(msg);
    }
    pImpl->cv.notify_one();
}