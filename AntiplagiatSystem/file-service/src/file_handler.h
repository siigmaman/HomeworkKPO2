#pragma once
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>
#include <string>
#include <memory>
#include "database.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class FileHandler {
private:
    http_listener listener;
    std::unique_ptr<Database> db;
    std::string upload_dir;
    
public:
    FileHandler(const std::string& url, const std::string& db_conn_str, const std::string& upload_dir);
    ~FileHandler();
    
    void start();
    void stop();
    
private:
    void handle_health(http_request request);
    void handle_upload(http_request request);
    void handle_get_file(http_request request);
    void handle_get_works(http_request request);
    void handle_get_reports(http_request request);
    void handle_options(http_request request);
    
    std::string calculate_file_hash(const std::string& filepath);
    std::string save_file(const concurrency::streams::istream& stream, const std::string& filename);
    void send_json_response(http_request request, status_code status, const json::value& body);
    void send_error_response(http_request request, status_code status, const std::string& error, const std::string& message);

    bool validate_upload_data(const json::value& data);
    std::string generate_filename(const std::string& original_name, const std::string& student_id);
};