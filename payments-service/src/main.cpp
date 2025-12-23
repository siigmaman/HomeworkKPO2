#include <iostream>
#include <thread>
#include <memory>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "payment_service.hpp"
#include "inbox_processor.hpp"
#include "outbox_processor.hpp"

using json = nlohmann::json;
using namespace httplib;

int main() {
    try {
        auto db = std::make_shared<Database>(
            std::getenv("DB_HOST") ?: "localhost",
            std::getenv("DB_PORT") ?: "5432",
            std::getenv("DB_NAME") ?: "payments_db",
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
        
        PaymentService payment_service(db);
        InboxProcessor inbox_processor(db, mq_config, payment_service);
        OutboxProcessor outbox_processor(db, mq_config);
        
        std::thread inbox_thread([&inbox_processor]() {
            inbox_processor.run();
        });
        
        std::thread outbox_thread([&outbox_processor]() {
            outbox_processor.run();
        });
        
        Server svr;
        
        svr.Post("/api/accounts", [&payment_service](const Request& req, Response& res) {
            try {
                auto json_body = json::parse(req.body);
                auto user_id = json_body["user_id"].get<std::string>();
                
                auto account = payment_service.create_account(user_id);
                res.set_content(account.to_json().dump(), "application/json");
                res.status = 201;
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 400;
            }
        });
        
        svr.Post("/api/accounts/:user_id/deposit", [&payment_service](const Request& req, Response& res) {
            try {
                auto user_id = req.path_params.at("user_id");
                auto json_body = json::parse(req.body);
                auto amount = json_body["amount"].get<double>();
                
                auto account = payment_service.deposit(user_id, amount);
                res.set_content(account.to_json().dump(), "application/json");
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 400;
            }
        });
        
        svr.Get("/api/accounts/:user_id/balance", [&payment_service](const Request& req, Response& res) {
            try {
                auto user_id = req.path_params.at("user_id");
                auto balance = payment_service.get_balance(user_id);
                
                json response = {{"user_id", user_id}, {"balance", balance}};
                res.set_content(response.dump(), "application/json");
            } catch (const std::exception& e) {
                json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
                res.status = 404;
            }
        });
        
        svr.Get("/health", [](const Request&, Response& res) {
            res.set_content("OK", "text/plain");
        });
        
        std::cout << "Payments Service starting on port 8080..." << std::endl;
        svr.listen("0.0.0.0", 8080);
        
        inbox_thread.join();
        outbox_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}