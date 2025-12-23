#ifndef NOTIFICATION_MANAGER_HPP
#define NOTIFICATION_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <set>
#include <mutex>
#include <nlohmann/json.hpp>

namespace websocket = boost::beast::websocket;
namespace asio = boost::asio;

class NotificationManager {
public:
    void subscribe(const std::string& order_id, websocket::stream<asio::ip::tcp::socket>* ws);
    void unsubscribe(const std::string& order_id, websocket::stream<asio::ip::tcp::socket>* ws);
    void notify(const std::string& order_id, const nlohmann::json& message);
    
private:
    std::unordered_map<std::string, std::set<websocket::stream<asio::ip::tcp::socket>*>> subscriptions_;
    std::mutex mutex_;
};

#endif