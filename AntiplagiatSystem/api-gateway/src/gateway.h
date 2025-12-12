#pragma once
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <string>
#include <map>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace web::http::client;

class APIGateway {
private:
    http_listener listener;

    std::string file_service_url;
    std::string analysis_service_url;

    http_client file_client;
    http_client analysis_client;
    
public:
    APIGateway(const std::string& url, 
               const std::string& file_service_url,
               const std::string& analysis_service_url);
    ~APIGateway();
    
    void start();
    void stop();
    
private:
    void handle_request(http_request request);
    void handle_health(http_request request);

    void route_to_file_service(http_request request);
    void route_to_analysis_service(http_request request);

    bool is_file_service_request(const std::string& path);
    bool is_analysis_service_request(const std::string& path);
    
    void send_response(http_request request, http_response response);
    void send_error_response(http_request request, status_code status,
                            const std::string& error, const std::string& message);
    
    void add_cors_headers(http_response& response);
};