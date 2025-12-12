#include "file_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <filesystem>

namespace fs = std::filesystem;

FileHandler::FileHandler(const std::string& url, const std::string& db_conn_str, const std::string& upload_dir)
    : listener(url), upload_dir(upload_dir) {
    
    try {
        db = std::make_unique<Database>(db_conn_str);

        listener.support(methods::GET, [this](http_request request) {
            auto path = request.relative_uri().path();
            
            if (path == U("/health")) {
                handle_health(request);
            } else if (path.find(U("/works/")) == 0 && path.size() > 7) {
                handle_get_works(request);
            } else if (path == U("/reports")) {
                handle_get_reports(request);
            } else if (path.find(U("/files/")) == 0) {
                handle_get_file(request);
            } else {
                send_error_response(request, status_codes::NotFound, 
                    "not_found", "Endpoint not found");
            }
        });
        
        listener.support(methods::POST, [this](http_request request) {
            auto path = request.relative_uri().path();
            
            if (path == U("/upload")) {
                handle_upload(request);
            } else {
                send_error_response(request, status_codes::NotFound,
                    "not_found", "Endpoint not found");
            }
        });
        
        listener.support(methods::OPTIONS, [this](http_request request) {
            handle_options(request);
        });

        fs::create_directories(upload_dir);
        
        std::cout << "[FILE SERVICE] Handler initialized" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Initialization: " << e.what() << std::endl;
        throw;
    }
}

FileHandler::~FileHandler() {
    stop();
}

void FileHandler::start() {
    try {
        listener.open().wait();
        std::cout << "[FILE SERVICE] Listening on: " << listener.uri().to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Starting: " << e.what() << std::endl;
        throw;
    }
}

void FileHandler::stop() {
    listener.close().wait();
    std::cout << "[FILE SERVICE] Stopped" << std::endl;
}

void FileHandler::handle_health(http_request request) {
    json::value response;
    response[U("service")] = json::value::string(U("file-service"));
    response[U("status")] = json::value::string(U("healthy"));
    response[U("database")] = json::value::boolean(db->is_connected());
    response[U("timestamp")] = json::value::string(
        utility::conversions::to_string_t(
            std::to_string(std::time(nullptr))
        )
    );
    
    send_json_response(request, status_codes::OK, response);
}

void FileHandler::handle_upload(http_request request) {
    try {
        request.extract_json().then([this, request](pplx::task<json::value> task) {
            try {
                auto data = task.get();

                if (!validate_upload_data(data)) {
                    send_error_response(request, status_codes::BadRequest,
                        "invalid_data", "Missing required fields");
                    return;
                }

                std::string student_id = data[U("student_id")].as_string();
                std::string student_name = data[U("student_name")].as_string();
                std::string assignment_id = data[U("assignment_id")].as_string();
                std::string assignment_name = data[U("assignment_name")].as_string();
                std::string file_content = data[U("file_content")].as_string();
                std::string original_filename = data[U("filename")].as_string();

                std::string filename = generate_filename(original_filename, student_id);
                std::string filepath = upload_dir + "/" + filename;

                std::ofstream file(filepath, std::ios::binary);
                if (!file) {
                    send_error_response(request, status_codes::InternalError,
                        "file_save_error", "Failed to save file");
                    return;
                }
                file.write(file_content.c_str(), file_content.size());
                file.close();

                std::string file_hash = calculate_file_hash(filepath);

                int work_id = db->save_work(student_id, student_name, assignment_id,
                                           assignment_name, filepath, original_filename, file_hash);

                json::value response;
                response[U("success")] = json::value::boolean(true);
                response[U("work_id")] = json::value::number(work_id);
                response[U("file_hash")] = json::value::string(
                    utility::conversions::to_string_t(file_hash));
                response[U("message")] = json::value::string(U("File uploaded successfully"));
                response[U("status")] = json::value::string(U("uploaded"));
                
                send_json_response(request, status_codes::Created, response);
                
            } catch (const std::exception& e) {
                std::cerr << "[FILE SERVICE ERROR] Upload: " << e.what() << std::endl;
                send_error_response(request, status_codes::InternalError,
                    "upload_error", e.what());
            }
        }).wait();
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Upload request: " << e.what() << std::endl;
        send_error_response(request, status_codes::BadRequest,
            "invalid_request", "Invalid request format");
    }
}

void FileHandler::handle_get_file(http_request request) {
    try {
        auto path = request.relative_uri().path();
        std::string path_str = utility::conversions::to_utf8string(path);

        size_t pos = path_str.find_last_of('/');
        if (pos == std::string::npos) {
            send_error_response(request, status_codes::BadRequest,
                "invalid_path", "Invalid file path");
            return;
        }
        
        int work_id = std::stoi(path_str.substr(pos + 1));

        std::string filepath = db->get_file_path(work_id);
        
        if (!fs::exists(filepath)) {
            send_error_response(request, status_codes::NotFound,
                "file_not_found", "File not found on server");
            return;
        }

        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            send_error_response(request, status_codes::InternalError,
                "file_read_error", "Failed to read file");
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.headers().add(U("Content-Type"), U("application/octet-stream"));
        response.set_body(content);
        request.reply(response);
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Get file: " << e.what() << std::endl;
        send_error_response(request, status_codes::InternalError,
            "internal_error", e.what());
    }
}

void FileHandler::handle_get_works(http_request request) {
    try {
        auto query = uri::split_query(request.request_uri().query());
        std::string assignment_id;

        for (const auto& param : query) {
            if (param.first == U("assignment_id")) {
                assignment_id = utility::conversions::to_utf8string(param.second);
                break;
            }
        }
        
        if (assignment_id.empty()) {
            send_error_response(request, status_codes::BadRequest,
                "missing_parameter", "assignment_id parameter is required");
            return;
        }

        auto result = db->get_works_by_assignment(assignment_id);

        json::value response;
        json::value works_array = json::value::array();
        
        int index = 0;
        for (const auto& row : result) {
            json::value work;
            work[U("id")] = json::value::number(row["id"].as<int>());
            work[U("student_id")] = json::value::string(
                utility::conversions::to_string_t(row["student_id"].as<std::string>()));
            work[U("student_name")] = json::value::string(
                utility::conversions::to_string_t(row["student_name"].as<std::string>()));
            work[U("filename")] = json::value::string(
                utility::conversions::to_string_t(row["original_filename"].as<std::string>()));
            work[U("upload_time")] = json::value::string(
                utility::conversions::to_string_t(row["upload_time"].as<std::string>()));
            work[U("status")] = json::value::string(
                utility::conversions::to_string_t(row["status"].as<std::string>()));
            
            works_array[index++] = work;
        }
        
        response[U("assignment_id")] = json::value::string(
            utility::conversions::to_string_t(assignment_id));
        response[U("count")] = json::value::number(result.size());
        response[U("works")] = works_array;
        
        send_json_response(request, status_codes::OK, response);
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Get works: " << e.what() << std::endl;
        send_error_response(request, status_codes::InternalError,
            "internal_error", e.what());
    }
}

void FileHandler::handle_get_reports(http_request request) {
    try {
        auto result = db->get_all_reports();
        
        json::value response;
        json::value reports_array = json::value::array();
        
        int index = 0;
        for (const auto& row : result) {
            json::value report;
            report[U("id")] = json::value::number(row["id"].as<int>());
            report[U("work_id")] = json::value::number(row["work_id"].as<int>());
            report[U("student_name")] = json::value::string(
                utility::conversions::to_string_t(row["student_name"].as<std::string>()));
            report[U("assignment_name")] = json::value::string(
                utility::conversions::to_string_t(row["assignment_name"].as<std::string>()));
            report[U("plagiarism_found")] = json::value::boolean(row["plagiarism_found"].as<bool>());
            report[U("similarity_percentage")] = json::value::number(
                row["similarity_percentage"].as<double>());
            report[U("matched_student_name")] = json::value::string(
                utility::conversions::to_string_t(row["matched_student_name"].as<std::string>()));
            report[U("analysis_time")] = json::value::string(
                utility::conversions::to_string_t(row["analysis_time"].as<std::string>()));
            
            reports_array[index++] = report;
        }
        
        response[U("count")] = json::value::number(result.size());
        response[U("reports")] = reports_array;
        
        send_json_response(request, status_codes::OK, response);
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE SERVICE ERROR] Get reports: " << e.what() << std::endl;
        send_error_response(request, status_codes::InternalError,
            "internal_error", e.what());
    }
}

void FileHandler::handle_options(http_request request) {
    http_response response(status_codes::OK);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
    request.reply(response);
}

std::string FileHandler::calculate_file_hash(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for hash calculation: " + filepath);
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize digest");
    }
    
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update digest");
        }
    }
    
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update digest");
        }
    }
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize digest");
    }
    
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool FileHandler::validate_upload_data(const json::value& data) {
    return data.has_field(U("student_id")) &&
           data.has_field(U("student_name")) &&
           data.has_field(U("assignment_id")) &&
           data.has_field(U("filename")) &&
           data.has_field(U("file_content"));
}

std::string FileHandler::generate_filename(const std::string& original_name, const std::string& student_id) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    size_t dot_pos = original_name.find_last_of('.');
    std::string extension = (dot_pos != std::string::npos) ? 
        original_name.substr(dot_pos) : "";

    return std::to_string(timestamp) + "_" + student_id + "_" + original_name;
}

void FileHandler::send_json_response(http_request request, status_code status, const json::value& body) {
    http_response response(status);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Content-Type"), U("application/json"));
    response.set_body(body);
    request.reply(response);
}

void FileHandler::send_error_response(http_request request, status_code status, 
                                     const std::string& error, const std::string& message) {
    json::value response;
    response[U("error")] = json::value::string(utility::conversions::to_string_t(error));
    response[U("message")] = json::value::string(utility::conversions::to_string_t(message));
    send_json_response(request, status, response);
}