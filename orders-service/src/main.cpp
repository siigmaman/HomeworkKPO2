#include <iostream>
#include <thread>
#include <memory>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "order_service.hpp"
#include "outbox_processor.hpp"

using json = nlohmann::json;
using namespace httplib;

int main() {
    try {
        auto db = std::make_shared<Database>(
            std::getenv("DB_HOST") ?: "localhost",
            std::getenv("DB_PORT") ?: "5432",
            std::getenv("DB_NAME") ?: "orders_db",
            std::getenv("DB_USER") ?: "microservice",
            std::getenv("DB_PASSWORD") ?: "password"
        );
        
        db->initialize_schema();
        
        auto mq_config = MessageQueueConfig{
            std::getenv("RABBITMQ_HOST") ?: "localhost",
            std::getenv("RABBITMQ_PORT") ?: "5672",
            std::getenv("RABBITMQ_USER") ?: "admin",
            std::getenv("RABBITMQ_PASS") ?: "password"
        };
        
        OrderService order_service(db, mq_config);
        OutboxProcessor outbox_processor(db, mq_config);
        std::thread outbox_thread([&outbox_processor]() {
            outbox_processor.run();
        });
        
        Server svr;
        
        svr.Post("/api/orders", [&order_service](const Request& req, Response& res) {
            try {
                auto json_body = json::parse(req.body);
                auto user_id = json_body["user_id"].get<std::string>();
                auto amount = json_body["amount"].get<double>();
                auto description = json_body["description"].get<std::string>();
                
                auto order = order_service.create_order(user_id, amount, description);
                res.set_content(order.to_json().dump(), "application/json");
                res.status = 201;
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 400;
            }
        });
        
        svr.Get("/api/orders", [&order_service](const Request& req, Response& res) {
            try {
                auto user_id = req.get_param_value("user_id");
                if (user_id.empty()) {
                    res.status = 400;
                    res.set_content("{\"error\":\"user_id is required\"}", "application/json");
                    return;
                }
                
                auto orders = order_service.get_user_orders(user_id);
                json orders_json = json::array();
                for (const auto& order : orders) {
                    orders_json.push_back(order.to_json());
                }
                
                res.set_content(orders_json.dump(), "application/json");
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        });
        
        svr.Get("/api/orders/:id", [&order_service](const Request& req, Response& res) {
            try {
                auto order_id = req.path_params.at("id");
                auto order = order_service.get_order(order_id);
                
                if (order.id.empty()) {
                    res.status = 404;
                    res.set_content("{\"error\":\"Order not found\"}", "application/json");
                    return;
                }
                
                res.set_content(order.to_json().dump(), "application/json");
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        });
        
        svr.Get("/health", [](const Request&, Response& res) {
            res.set_content("OK", "text/plain");
        });
        
        std::cout << "Orders Service starting on port 8080..." << std::endl;
        svr.listen("0.0.0.0", 8080);
        
        outbox_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}