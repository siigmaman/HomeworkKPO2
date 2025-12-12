#include "gateway.h"
#include <iostream>
#include <regex>
#include <ctime>

using namespace utility;

APIGateway::APIGateway(const std::string& url,
                       const std::string& file_service_url,
                       const std::string& analysis_service_url)
    : listener(conversions::to_string_t(url)),
      file_service_url(file_service_url),
      analysis_service_url(analysis_service_url),
      file_client(conversions::to_string_t(file_service_url)),
      analysis_client(conversions::to_string_t(analysis_service_url)) {

    listener.support([this](http_request request) {
        handle_request(request);
    });
    
    std::cout << "[API GATEWAY] Initialized" << std::endl;
    std::cout << "[API GATEWAY] File Service: " << file_service_url << std::endl;
    std::cout << "[API GATEWAY] Analysis Service: " << analysis_service_url << std::endl;
}

APIGateway::~APIGateway() {
    stop();
}

void APIGateway::start() {
    try {
        listener.open().wait();
        std::cout << "[API GATEWAY] Listening on: " << conversions::to_utf8string(listener.uri().to_string()) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[API GATEWAY ERROR] Starting: " << e.what() << std::endl;
        throw;
    }
}

void APIGateway::stop() {
    listener.close().wait();
    std::cout << "[API GATEWAY] Stopped" << std::endl;
}

void APIGateway::handle_request(http_request request) {
    try {
        auto path = request.relative_uri().path();
        std::string path_str = conversions::to_utf8string(path);
        
        std::cout << "[API GATEWAY] Request: " << request.method() << " " << path_str << std::endl;

        if (path == U("/health")) {
            handle_health(request);
            return;
        }

        if (request.method() == methods::OPTIONS) {
            http_response response(status_codes::OK);
            add_cors_headers(response);
            request.reply(response);
            return;
        }

        if (is_file_service_request(path_str)) {
            route_to_file_service(request);
        } else if (is_analysis_service_request(path_str)) {
            route_to_analysis_service(request);
        } else {
            send_error_response(request, status_codes::NotFound,
                "not_found", "Endpoint not found");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[API GATEWAY ERROR] Handle request: " << e.what() << std::endl;
        send_error_response(request, status_codes::InternalError,
            "gateway_error", "Internal gateway error");
    }
}

void APIGateway::handle_health(http_request request) {
    json::value response;
    response[U("service")] = json::value::string(U("api-gateway"));
    response[U("status")] = json::value::string(U("healthy"));
    response[U("timestamp")] = json::value::string(
        conversions::to_string_t(std::to_string(std::time(nullptr)))
    );

    try {
        auto file_response = file_client.request(methods::GET, U("/health")).get();
        response[U("file_service")] = json::value::string(
            file_response.status_code() == status_codes::OK ? U("healthy") : U("unhealthy"));
    } catch (...) {
        response[U("file_service")] = json::value::string(U("unreachable"));
    }
    
    try {
        auto analysis_response = analysis_client.request(methods::GET, U("/health")).get();
        response[U("analysis_service")] = json::value::string(
            analysis_response.status_code() == status_codes::OK ? U("healthy") : U("unhealthy"));
    } catch (...) {
        response[U("analysis_service")] = json::value::string(U("unreachable"));
    }
    
    http_response http_resp(status_codes::OK);
    http_resp.set_body(response);
    http_resp.headers().add(U("Content-Type"), U("application/json"));
    send_response(request, http_resp);
}

void APIGateway::route_to_file_service(http_request request) {
    try {
        auto target_path = request.relative_uri().path();

        std::string path_str = conversions::to_utf8string(target_path);
        if (path_str.find("/api/files") == 0) {
            path_str = path_str.substr(4); 
        }
        
        http_request proxy_request(request.method());
        proxy_request.set_request_uri(conversions::to_string_t(path_str));

        for (const auto& header : request.headers()) {
            proxy_request.headers().add(header.first, header.second);
        }

        if (request.method() == methods::POST || request.method() == methods::PUT) {
            proxy_request.set_body(request.extract_json().get());
        }

        auto response = file_client.request(proxy_request).get();

        send_response(request, response);
        
    } catch (const std::exception& e) {
        std::cerr << "[API GATEWAY ERROR] File service routing: " << e.what() << std::endl;
        send_error_response(request, status_codes::BadGateway,
            "service_unavailable", "File service is unavailable");
    }
}

void APIGateway::route_to_analysis_service(http_request request) {
    try {
        auto target_path = request.relative_uri().path();

        std::string path_str = conversions::to_utf8string(target_path);
        if (path_str.find("/api/reports") == 0) {
            path_str = path_str.substr(4);
        } else if (path_str.find("/api/analyze") == 0) {
            path_str = "/analyze";
        }
        
        http_request proxy_request(request.method());
        proxy_request.set_request_uri(conversions::to_string_t(path_str));

        for (const auto& header : request.headers()) {
            proxy_request.headers().add(header.first, header.second);
        }

        if (request.method() == methods::POST) {
            proxy_request.set_body(request.extract_json().get());
        }

        auto response = analysis_client.request(proxy_request).get();

        send_response(request, response);
        
    } catch (const std::exception& e) {
        std::cerr << "[API GATEWAY ERROR] Analysis service routing: " << e.what() << std::endl;
        send_error_response(request, status_codes::BadGateway,
            "service_unavailable", "Analysis service is unavailable");
    }
}

bool APIGateway::is_file_service_request(const std::string& path) {
    static const std::regex file_patterns[] = {
        std::regex("^/api/files/.*"),
        std::regex("^/api/upload"),
        std::regex("^/api/works")
    };
    
    for (const auto& pattern : file_patterns) {
        if (std::regex_match(path, pattern)) {
            return true;
        }
    }
    
    return false;
}

bool APIGateway::is_analysis_service_request(const std::string& path) {
    static const std::regex analysis_patterns[] = {
        std::regex("^/api/reports/.*"),
        std::regex("^/api/analyze")
    };
    
    for (const auto& pattern : analysis_patterns) {
        if (std::regex_match(path, pattern)) {
            return true;
        }
    }
    
    return false;
}

void APIGateway::send_response(http_request request, http_response response) {
    add_cors_headers(response);
    request.reply(response);
}

void APIGateway::send_error_response(http_request request, status_code status,
                                    const std::string& error, const std::string& message) {
    json::value response;
    response[U("error")] = json::value::string(conversions::to_string_t(error));
    response[U("message")] = json::value::string(conversions::to_string_t(message));
    
    http_response http_resp(status);
    http_resp.set_body(response);
    http_resp.headers().add(U("Content-Type"), U("application/json"));
    
    add_cors_headers(http_resp);
    request.reply(http_resp);
}

void APIGateway::add_cors_headers(http_response& response) {
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    response.headers().add(U("Access-Control-Max-Age"), U("86400"));
}