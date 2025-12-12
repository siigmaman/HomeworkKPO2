#include "database.h"
#include <iostream>

Database::Database(const std::string& connection_string) {
    try {
        conn = std::make_unique<pqxx::connection>(connection_string);
        if (conn->is_open()) {
            std::cout << "[ANALYSIS DB] Connected to PostgreSQL successfully" << std::endl;
        } else {
            throw std::runtime_error("Cannot open database connection");
        }
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] " << e.what() << std::endl;
        throw;
    }
}

Database::~Database() {
    if (conn && conn->is_open()) {
        conn->close();
        std::cout << "[ANALYSIS DB] Connection closed" << std::endl;
    }
}

bool Database::is_connected() const {
    return conn && conn->is_open();
}

Database::WorkInfo Database::get_work_info(int work_id) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT id, student_id, student_name, assignment_id, file_path, file_hash, status "
            "FROM works WHERE id = $1",
            work_id
        );
        
        if (result.empty()) {
            throw std::runtime_error("Work not found with ID: " + std::to_string(work_id));
        }
        
        WorkInfo info;
        info.id = result[0]["id"].as<int>();
        info.student_id = result[0]["student_id"].as<std::string>();
        info.student_name = result[0]["student_name"].as<std::string>();
        info.assignment_id = result[0]["assignment_id"].as<std::string>();
        info.file_path = result[0]["file_path"].as<std::string>();
        info.file_hash = result[0]["file_hash"].as<std::string>();
        info.status = result[0]["status"].as<std::string>();
        
        return info;
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] get_work_info: " << e.what() << std::endl;
        throw;
    }
}

std::vector<Database::SimilarWork> Database::find_similar_works(const std::string& file_hash, 
                                                               const std::string& exclude_student_id) {
    std::vector<SimilarWork> similar_works;
    
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT id, student_id, student_name, file_hash FROM works "
            "WHERE file_hash = $1 AND student_id != $2 AND status != 'duplicate_detected'",
            file_hash, exclude_student_id
        );
        
        for (const auto& row : result) {
            SimilarWork work;
            work.id = row["id"].as<int>();
            work.student_id = row["student_id"].as<std::string>();
            work.student_name = row["student_name"].as<std::string>();
            work.file_hash = row["file_hash"].as<std::string>();
            similar_works.push_back(work);
        }
        
        return similar_works;
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] find_similar_works: " << e.what() << std::endl;
        return similar_works;
    }
}

void Database::save_report(int work_id,
                          bool plagiarism_found,
                          double similarity_percentage,
                          int matched_work_id,
                          const std::string& matched_student_name,
                          const std::string& report_data) {
    try {
        pqxx::work txn(*conn);

        txn.exec_params("DELETE FROM reports WHERE work_id = $1", work_id);

        txn.exec_params(
            "INSERT INTO reports (work_id, plagiarism_found, similarity_percentage, "
            "matched_work_id, matched_student_name, report_data) "
            "VALUES ($1, $2, $3, $4, $5, $6)",
            work_id,
            plagiarism_found,
            similarity_percentage,
            matched_work_id,
            matched_student_name,
            report_data
        );

        std::string status = plagiarism_found ? "plagiarism_found" : "checked_ok";
        txn.exec_params(
            "UPDATE works SET status = $1 WHERE id = $2",
            status, work_id
        );
        
        txn.commit();
        std::cout << "[ANALYSIS DB] Report saved for work ID: " << work_id << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] save_report: " << e.what() << std::endl;
        throw;
    }
}

bool Database::report_exists(int work_id) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT COUNT(*) as count FROM reports WHERE work_id = $1",
            work_id
        );
        
        return result[0]["count"].as<int>() > 0;
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] report_exists: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::get_report(int work_id) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT report_data::text as report FROM reports WHERE work_id = $1",
            work_id
        );
        
        if (result.empty()) {
            return "{}";
        }
        
        return result[0]["report"].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] get_report: " << e.what() << std::endl;
        return "{}";
    }
}

void Database::update_work_status(int work_id, const std::string& status) {
    try {
        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE works SET status = $1 WHERE id = $2",
            status, work_id
        );
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "[ANALYSIS DB ERROR] update_work_status: " << e.what() << std::endl;
        throw;
    }
}