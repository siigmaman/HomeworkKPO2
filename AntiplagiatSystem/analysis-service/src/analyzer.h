#pragma once
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <string>
#include <memory>
#include "database.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Analyzer {
private:
    http_listener listener;
    std::unique_ptr<Database> db;
    std::string file_service_url;
    
public:
    Analyzer(const std::string& url, const std::string& db_conn_str, 
             const std::string& file_service_url);
    ~Analyzer();
    
    void start();
    void stop();
    
private:
    void handle_health(http_request request);
    void handle_analyze(http_request request);
    void handle_get_report(http_request request);
    void handle_options(http_request request);

    bool check_plagiarism_simple(const std::string& file_hash, 
                                 const std::string& student_id,
                                 Database::SimilarWork& match);
    
    double calculate_similarity(const std::string& filepath1, 
                                const std::string& filepath2);
    
    std::string read_file_content(const std::string& filepath);
    std::string normalize_text(const std::string& text);

    void send_json_response(http_request request, status_code status, const json::value& body);
    void send_error_response(http_request request, status_code status, 
                            const std::string& error, const std::string& message);
    
    json::value create_report_json(bool plagiarism_found, double similarity,
                                  const Database::SimilarWork& match);
};