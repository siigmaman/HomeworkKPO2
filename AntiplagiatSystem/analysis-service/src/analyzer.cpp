#include "analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cpprest/http_client.h>

using namespace web::http;
using namespace web::http::client;

Analyzer::Analyzer(const std::string& url, const std::string& db_conn_str,
                   const std::string& file_service_url)
    : listener(url), file_service_url(file_service_url) {
    
    try {
        db = std::make_unique<Database>(db_conn_str);

        listener.support(methods::GET, [this](http_request request) {
            auto path = request.relative_uri().path();
            
            if (path == U("/health")) {
                handle_health(request);
            } else if (path.find(U("/reports/")) == 0) {
                handle_get_report(request);
            } else {
                send_error_response(request, status_codes::NotFound,
                    "not_found", "Endpoint not found");
            }
        });
        
        listener.support(methods::POST, [this](http_request request) {
            auto path = request.relative_uri().path();
            
            if (path == U("/analyze")) {
                handle_analyze(request);
            } else {
                send_error_response(request, status_codes::NotFound,
                    "not_found", "Endpoint not found");
            }
        });
        
        listener.support(methods::OPTIONS, [this](http_request request) {
            handle_options(request);
        });
        
        std::cout << "[ANALYSIS SERVICE] Handler initialized" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS SERVICE ERROR] Initialization: " << e.what() << std::endl;
        throw;
    }
}

Analyzer::~Analyzer() {
    stop();
}

void Analyzer::start() {
    try {
        listener.open().wait();
        std::cout << "[ANALYSIS SERVICE] Listening on: " << listener.uri().to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS SERVICE ERROR] Starting: " << e.what() << std::endl;
        throw;
    }
}

void Analyzer::stop() {
    listener.close().wait();
    std::cout << "[ANALYSIS SERVICE] Stopped" << std::endl;
}

void Analyzer::handle_health(http_request request) {
    json::value response;
    response[U("service")] = json::value::string(U("analysis-service"));
    response[U("status")] = json::value::string(U("healthy"));
    response[U("database")] = json::value::boolean(db->is_connected());
    response[U("file_service")] = json::value::string(
        utility::conversions::to_string_t(file_service_url));
    response[U("timestamp")] = json::value::string(
        utility::conversions::to_string_t(
            std::to_string(std::time(nullptr))
        )
    );
    
    send_json_response(request, status_codes::OK, response);
}

void Analyzer::handle_analyze(http_request request) {
    try {
        request.extract_json().then([this, request](pplx::task<json::value> task) {
            try {
                auto data = task.get();
                
                if (!data.has_field(U("work_id"))) {
                    send_error_response(request, status_codes::BadRequest,
                        "missing_field", "work_id is required");
                    return;
                }
                
                int work_id = data[U("work_id")].as_integer();
                std::cout << "[ANALYSIS] Starting analysis for work ID: " << work_id << std::endl;

                auto work_info = db->get_work_info(work_id);

                Database::SimilarWork match;
                bool plagiarism_found = check_plagiarism_simple(
                    work_info.file_hash, work_info.student_id, match);
                
                double similarity = 0.0;
                if (plagiarism_found) {
                    similarity = 100.0; 
                }

                auto report_json = create_report_json(plagiarism_found, similarity, match);
                std::string report_str = utility::conversions::to_utf8string(
                    report_json.serialize());

                std::string matched_name = plagiarism_found ? match.student_name : "";
                int matched_id = plagiarism_found ? match.id : -1;
                
                db->save_report(work_id, plagiarism_found, similarity,
                               matched_id, matched_name, report_str);

                json::value response;
                response[U("success")] = json::value::boolean(true);
                response[U("work_id")] = json::value::number(work_id);
                response[U("plagiarism_found")] = json::value::boolean(plagiarism_found);
                response[U("similarity_percentage")] = json::value::number(similarity);
                
                if (plagiarism_found) {
                    response[U("matched_work_id")] = json::value::number(match.id);
                    response[U("matched_student_name")] = json::value::string(
                        utility::conversions::to_string_t(match.student_name));
                }
                
                response[U("message")] = json::value::string(
                    plagiarism_found ? 
                    U("Plagiarism detected") : 
                    U("No plagiarism detected"));
                
                send_json_response(request, status_codes::OK, response);
                
            } catch (const std::exception& e) {
                std::cerr << "[ANALYSIS SERVICE ERROR] Analyze: " << e.what() << std::endl;
                send_error_response(request, status_codes::InternalError,
                    "analysis_error", e.what());
            }
        }).wait();
        
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS SERVICE ERROR] Analyze request: " << e.what() << std::endl;
        send_error_response(request, status_codes::BadRequest,
            "invalid_request", "Invalid request format");
    }
}

void Analyzer::handle_get_report(http_request request) {
    try {
        auto path = request.relative_uri().path();
        std::string path_str = utility::conversions::to_utf8string(path);

        size_t pos = path_str.find_last_of('/');
        if (pos == std::string::npos) {
            send_error_response(request, status_codes::BadRequest,
                "invalid_path", "Invalid report path");
            return;
        }
        
        int work_id = std::stoi(path_str.substr(pos + 1));

        if (!db->report_exists(work_id)) {
            send_error_response(request, status_codes::NotFound,
                "report_not_found", "Report not found for this work");
            return;
        }

        std::string report_json = db->get_report(work_id);

        json::value report = json::value::parse(
            utility::conversions::to_string_t(report_json));
        
        json::value response;
        response[U("work_id")] = json::value::number(work_id);
        response[U("report")] = report;
        
        send_json_response(request, status_codes::OK, response);
        
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS SERVICE ERROR] Get report: " << e.what() << std::endl;
        send_error_response(request, status_codes::InternalError,
            "internal_error", e.what());
    }
}

bool Analyzer::check_plagiarism_simple(const std::string& file_hash, 
                                      const std::string& student_id,
                                      Database::SimilarWork& match) {
    auto similar_works = db->find_similar_works(file_hash, student_id);
    
    if (!similar_works.empty()) {
        match = similar_works[0];
        return true;
    }
    
    return false;
}

double Analyzer::calculate_similarity(const std::string& filepath1, 
                                     const std::string& filepath2) {
    try {
        std::string content1 = read_file_content(filepath1);
        std::string content2 = read_file_content(filepath2);
        
        if (content1.empty() || content2.empty()) {
            return 0.0;
        }

        std::string normalized1 = normalize_text(content1);
        std::string normalized2 = normalize_text(content2);
        
        if (normalized1 == normalized2) {
            return 100.0;
        }
        
        return 0.0;
        
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS ERROR] calculate_similarity: " << e.what() << std::endl;
        return 0.0;
    }
}

std::string Analyzer::read_file_content(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Analyzer::normalize_text(const std::string& text) {
    std::string result = text;

    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    result.erase(std::unique(result.begin(), result.end(),
        [](char a, char b) { return std::isspace(a) && std::isspace(b); }),
        result.end());
    
    return result;
}

json::value Analyzer::create_report_json(bool plagiarism_found, double similarity,
                                        const Database::SimilarWork& match) {
    json::value report;
    report[U("analysis_timestamp")] = json::value::string(
        utility::conversions::to_string_t(
            std::to_string(std::time(nullptr))
        )
    );
    report[U("plagiarism_found")] = json::value::boolean(plagiarism_found);
    report[U("similarity_percentage")] = json::value::number(similarity);
    
    if (plagiarism_found) {
        report[U("match_details")] = json::value::object();
        report[U("match_details")][U("work_id")] = json::value::number(match.id);
        report[U("match_details")][U("student_name")] = json::value::string(
            utility::conversions::to_string_t(match.student_name));
        report[U("match_details")][U("matching_hash")] = json::value::string(
            utility::conversions::to_string_t(match.file_hash));
    }
    
    report[U("algorithm_used")] = json::value::string(U("simple_hash_comparison"));
    
    return report;
}

void Analyzer::handle_options(http_request request) {
    http_response response(status_codes::OK);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
    request.reply(response);
}

void Analyzer::send_json_response(http_request request, status_code status, const json::value& body) {
    http_response response(status);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Content-Type"), U("application/json"));
    response.set_body(body);
    request.reply(response);
}

void Analyzer::send_error_response(http_request request, status_code status,
                                  const std::string& error, const std::string& message) {
    json::value response;
    response[U("error")] = json::value::string(utility::conversions::to_string_t(error));
    response[U("message")] = json::value::string(utility::conversions::to_string_t(message));
    send_json_response(request, status, response);
}