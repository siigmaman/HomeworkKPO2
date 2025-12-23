#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class NotificationManager;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    WebSocketSession(tcp::socket socket, NotificationManager& notification_manager);
    void start();
    
private:
    void on_accept(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);
    
    websocket::stream<tcp::socket> ws_;
    NotificationManager& notification_manager_;
    beast::flat_buffer buffer_;
    std::string order_id_;
};

class WebSocketServer {
public:
    WebSocketServer(asio::io_context& ioc, NotificationManager& notification_manager);
    void run(const std::string& address, unsigned short port);
    
private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
    
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    NotificationManager& notification_manager_;
};

#endif