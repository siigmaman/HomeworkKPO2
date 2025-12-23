#include "websocket_server.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebSocketSession::WebSocketSession(tcp::socket socket, 
                                   NotificationManager& notification_manager)
    : ws_(std::move(socket)), notification_manager_(notification_manager) {
}

void WebSocketSession::start() {
    ws_.async_accept(
        [self = shared_from_this()](beast::error_code ec) {
            self->on_accept(ec);
        });
}

void WebSocketSession::on_accept(beast::error_code ec) {
    if (ec) return;
    do_read();
}

void WebSocketSession::do_read() {
    ws_.async_read(
        buffer_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
            self->on_read(ec, bytes_transferred);
        });
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t) {
    if (ec == websocket::error::closed) {
        if (!order_id_.empty()) {
            notification_manager_.unsubscribe(order_id_, &ws_);
        }
        return;
    }
    
    if (ec) return;
    
    try {
        auto message = beast::buffers_to_string(buffer_.data());
        auto json_msg = json::parse(message);
        
        if (json_msg["type"] == "subscribe" && json_msg.contains("order_id")) {
            order_id_ = json_msg["order_id"].get<std::string>();
            notification_manager_.subscribe(order_id_, &ws_);
            
            json response = {
                {"type", "subscribed"},
                {"order_id", order_id_}
            };
            ws_.write(asio::buffer(response.dump()));
        }
        
    } catch (...) {
    }
    
    buffer_.consume(buffer_.size());
    do_read();
}

void WebSocketSession::on_close(beast::error_code ec) {
    if (!order_id_.empty()) {
        notification_manager_.unsubscribe(order_id_, &ws_);
    }
}

WebSocketServer::WebSocketServer(asio::io_context& ioc, 
                                 NotificationManager& notification_manager)
    : ioc_(ioc), acceptor_(ioc), notification_manager_(notification_manager) {
}

void WebSocketServer::run(const std::string& address, unsigned short port) {
    tcp::endpoint endpoint(asio::ip::make_address(address), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    
    do_accept();
}

void WebSocketServer::do_accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            on_accept(ec, std::move(socket));
        });
}

void WebSocketServer::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) return;
    
    std::make_shared<WebSocketSession>(std::move(socket), notification_manager_)->start();
    
    do_accept();
}