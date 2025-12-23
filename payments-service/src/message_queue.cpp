#include "message_queue.hpp"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_ssl_socket.h>
#include <iostream>

MessageQueue::MessageQueue(const MessageQueueConfig& config) {
    connection_ = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(connection_);
    
    if (!socket) {
        throw std::runtime_error("Cannot create TCP socket");
    }
    
    int status = amqp_socket_open(socket, config.host.c_str(), std::stoi(config.port));
    if (status) {
        throw std::runtime_error("Cannot open socket");
    }
    
    amqp_rpc_reply_t reply = amqp_login(connection_, "/", 0, 131072, 0, 
                                        AMQP_SASL_METHOD_PLAIN, 
                                        config.user.c_str(), 
                                        config.password.c_str());
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("Cannot login to RabbitMQ");
    }
    
    channel_ = amqp_channel_new(connection_);
    if (!channel_) {
        throw std::runtime_error("Cannot create channel");
    }
    
    amqp_channel_open(connection_, 1);
    reply = amqp_get_rpc_reply(connection_);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        throw std::runtime_error("Cannot open channel");
    }
}

MessageQueue::~MessageQueue() {
    if (channel_) {
        amqp_channel_close(connection_, 1, AMQP_REPLY_SUCCESS);
    }
    if (connection_) {
        amqp_connection_close(connection_, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(connection_);
    }
}

void MessageQueue::publish(const std::string& queue, const std::string& message) {
    amqp_bytes_t queue_bytes = amqp_cstring_bytes(queue.c_str());
    amqp_bytes_t message_bytes = amqp_cstring_bytes(message.c_str());
    
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.delivery_mode = 2;
    
    int result = amqp_basic_publish(connection_, 1, amqp_cstring_bytes(""),
                                    queue_bytes, 0, 0, &props, message_bytes);
    if (result < 0) {
        throw std::runtime_error("Failed to publish message");
    }
}

void MessageQueue::consume(const std::string& queue, 
                           std::function<void(const std::string&)> callback) {
    amqp_bytes_t queue_bytes = amqp_cstring_bytes(queue.c_str());
    
    amqp_queue_declare(connection_, 1, queue_bytes, 0, 1, 0, 0, amqp_empty_table);
    amqp_basic_consume(connection_, 1, queue_bytes, amqp_empty_bytes, 0, 1, 0, 
                       amqp_empty_table);
    
    while (true) {
        amqp_envelope_t envelope;
        amqp_maybe_release_buffers(connection_);
        
        amqp_rpc_reply_t ret = amqp_consume_message(connection_, &envelope, nullptr, 0);
        if (ret.reply_type == AMQP_RESPONSE_NORMAL) {
            std::string message(static_cast<char*>(envelope.message.body.bytes),
                               envelope.message.body.len);
            callback(message);
            amqp_destroy_envelope(&envelope);
        }
    }
}