#include <iostream>
#include <memory>
#include <unordered_map>
#include <set>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "message_queue.hpp"
#include "websocket_server.hpp"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

class NotificationManager {
public:
    void subscribe(const std::string& order_id, websocket::stream<tcp::socket>* ws) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscriptions_[order_id].insert(ws);
    }
    
    void unsubscribe(const std::string& order_id, websocket::stream<tcp::socket>* ws) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(order_id);
        if (it != subscriptions_.end()) {
            it->second.erase(ws);
            if (it->second.empty()) {
                subscriptions_.erase(it);
            }
        }
    }
    
    void notify(const std::string& order_id, const json& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(order_id);
        if (it != subscriptions_.end()) {
            auto msg = message.dump();
            for (auto* ws : it->second) {
                try {
                    ws->write(asio::buffer(msg));
                } catch (...) {
                }
            }
        }
    }
    
private:
    std::unordered_map<std::string, std::set<websocket::stream<tcp::socket>*>> subscriptions_;
    std::mutex mutex_;
};

int main() {
    try {
        asio::io_context ioc;
        
        auto mq_config = MessageQueueConfig{
            std::getenv("RABBITMQ_HOST") ?: "localhost",
            std::getenv("RABBITMQ_PORT") ?: "5672",
            std::getenv("RABBITMQ_USER") ?: "admin",
            std::getenv("RABBITMQ_PASS") ?: "password"
        };
        
        NotificationManager notification_manager;
        
        auto message_queue = std::make_unique<MessageQueue>(mq_config);
        message_queue->consume("payment.results", [&notification_manager](const std::string& message) {
            try {
                auto json_msg = json::parse(message);
                auto order_id = json_msg["order_id"].get<std::string>();
                json notification = {
                    {"type", "order_update"},
                    {"order_id", order_id},
                    {"status", json_msg["success"] ? "FINISHED" : "CANCELLED"},
                    {"message", json_msg["message"]},
                    {"timestamp", std::time(nullptr)}
                };
                notification_manager.notify(order_id, notification);
            } catch (...) {
            }
        });
        
        auto server = std::make_shared<WebSocketServer>(ioc, notification_manager);
        server->run("0.0.0.0", 8080);
        
        std::cout << "WebSocket Service starting on port 8080..." << std::endl;
        ioc.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}