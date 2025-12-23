#include "notification_manager.hpp"
#include <mutex>
#include <set>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void NotificationManager::subscribe(const std::string& order_id, 
                                    websocket::stream<tcp::socket>* ws) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[order_id].insert(ws);
}

void NotificationManager::unsubscribe(const std::string& order_id, 
                                      websocket::stream<tcp::socket>* ws) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscriptions_.find(order_id);
    if (it != subscriptions_.end()) {
        it->second.erase(ws);
        if (it->second.empty()) {
            subscriptions_.erase(it);
        }
    }
}

void NotificationManager::notify(const std::string& order_id, 
                                 const nlohmann::json& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscriptions_.find(order_id);
    if (it != subscriptions_.end()) {
        auto msg = message.dump();
        for (auto* ws : it->second) {
            try {
                ws->write(boost::asio::buffer(msg));
            } catch (...) {
            }
        }
    }
}