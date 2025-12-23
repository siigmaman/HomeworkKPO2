#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

struct Order {
    std::string id;
    std::string user_id;
    double amount;
    std::string description;
    std::string status;
    std::chrono::system_clock::time_point created_at;
    
    static Order from_json(const json& j) {
        return Order{
            j["id"].get<std::string>(),
            j["user_id"].get<std::string>(),
            j["amount"].get<double>(),
            j["description"].get<std::string>(),
            j["status"].get<std::string>(),
            std::chrono::system_clock::now()
        };
    }
    
    json to_json() const {
        return {
            {"id", id},
            {"user_id", user_id},
            {"amount", amount},
            {"description", description},
            {"status", status},
            {"created_at", std::chrono::system_clock::to_time_t(created_at)}
        };
    }
};

struct Account {
    std::string user_id;
    double balance;
    int version;
    
    json to_json() const {
        return {
            {"user_id", user_id},
            {"balance", balance},
            {"version", version}
        };
    }
};

struct OutboxEvent {
    std::string id;
    std::string type;
    json payload;
    std::string status;
    std::chrono::system_clock::time_point created_at;
};

struct InboxEvent {
    std::string id;
    std::string type;
    json payload;
    std::string status;
    std::chrono::system_clock::time_point processed_at;
};

namespace messages {

struct PaymentRequest {
    std::string order_id;
    std::string user_id;
    double amount;
    
    static PaymentRequest from_json(const json& j) {
        return PaymentRequest{
            j["order_id"].get<std::string>(),
            j["user_id"].get<std::string>(),
            j["amount"].get<double>()
        };
    }
    
    json to_json() const {
        return {
            {"order_id", order_id},
            {"user_id", user_id},
            {"amount", amount}
        };
    }
};

struct PaymentResult {
    std::string order_id;
    std::string user_id;
    bool success;
    std::string message;
    
    json to_json() const {
        return {
            {"order_id", order_id},
            {"user_id", user_id},
            {"success", success},
            {"message", message}
        };
    }
};

}
}

#endif