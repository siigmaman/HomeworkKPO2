#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "gateway.h"

int main() {
    std::cout << "=== API Gateway Starting ===" << std::endl;

    std::string file_service_url = std::getenv("FILE_SERVICE_URL") ? 
        std::getenv("FILE_SERVICE_URL") : "http://localhost:8081";
    std::string analysis_service_url = std::getenv("ANALYSIS_SERVICE_URL") ? 
        std::getenv("ANALYSIS_SERVICE_URL") : "http://localhost:8082";
    std::string gateway_port = std::getenv("GATEWAY_PORT") ? 
        std::getenv("GATEWAY_PORT") : "8080";
    
    std::cout << "File Service URL: " << file_service_url << std::endl;
    std::cout << "Analysis Service URL: " << analysis_service_url << std::endl;
    std::cout << "Gateway port: " << gateway_port << std::endl;
    
    try {
        APIGateway gateway("http://0.0.0.0:" + gateway_port,
                          file_service_url,
                          analysis_service_url);

        gateway.start();
        
        std::cout << "=== API Gateway is Running ===" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "!!! FATAL ERROR !!!" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}