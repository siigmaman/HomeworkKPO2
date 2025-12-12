#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "file_handler.h"

int main() {
    std::cout << "=== File Storage Service Starting ===" << std::endl;

    std::string db_host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
    std::string db_port = std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432";
    std::string db_name = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "antiplagiat";
    std::string db_user = std::getenv("DB_USER") ? std::getenv("DB_USER") : "admin";
    std::string db_pass = std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "secret";
    std::string upload_dir = std::getenv("UPLOAD_DIR") ? std::getenv("UPLOAD_DIR") : "/app/uploads";
    std::string service_port = std::getenv("SERVICE_PORT") ? std::getenv("SERVICE_PORT") : "8080";

    std::string conn_str = "host=" + db_host + 
                          " port=" + db_port + 
                          " dbname=" + db_name + 
                          " user=" + db_user + 
                          " password=" + db_pass;
    
    std::cout << "Database connection: " << db_host << ":" << db_port << "/" << db_name << std::endl;
    std::cout << "Upload directory: " << upload_dir << std::endl;
    std::cout << "Service port: " << service_port << std::endl;
    
    try {
        FileHandler handler("http://0.0.0.0:" + service_port, conn_str, upload_dir);

        handler.start();
        
        std::cout << "=== File Service is Running ===" << std::endl;
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