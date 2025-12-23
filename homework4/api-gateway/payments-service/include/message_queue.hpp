#ifndef PAYMENTS_MESSAGE_QUEUE_HPP
#define PAYMENTS_MESSAGE_QUEUE_HPP

#include <string>
#include <functional>

struct MessageQueueConfig {
    std::string host;
    std::string port;
    std::string user;
    std::string password;
};

class MessageQueue {
public:
    MessageQueue(const MessageQueueConfig& config);
    ~MessageQueue();
    
    void publish(const std::string& queue, const std::string& message);
    void consume(const std::string& queue, std::function<void(const std::string&)> callback);
    
private:
    void* connection_;
    void* channel_;
};

#endif