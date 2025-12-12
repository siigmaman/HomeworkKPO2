#include "database.h"
#include <iostream>
#include <sstream>

Database::Database(const std::string& connection_string) {
    try {
        conn = std::make_unique<pqxx::connection>(connection_string);
        if (conn->is_open()) {
            std::cout << "[DATABASE] Connected to PostgreSQL successfully" << std::endl;
            create_tables();
        } else {
            throw std::runtime_error("Cannot open database connection");
        }
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] " << e.what() << std::endl;
        throw;
    }
}

Database::~Database() {
    if (conn && conn->is_open()) {
        conn->close();
        std::cout << "[DATABASE] Connection closed" << std::endl;
    }
}

bool Database::is_connected() const {
    return conn && conn->is_open();
}

void Database::create_tables() {
    try {
        pqxx::work txn(*conn);
        
        txn.exec("CREATE TABLE IF NOT EXISTS works ("
                 "id SERIAL PRIMARY KEY,"
                 "student_id VARCHAR(100) NOT NULL,"
                 "student_name VARCHAR(255) NOT NULL,"
                 "assignment_id VARCHAR(100) NOT NULL,"
                 "assignment_name VARCHAR(255),"
                 "file_path VARCHAR(500) NOT NULL,"
                 "original_filename VARCHAR(255) NOT NULL,"
                 "file_hash VARCHAR(64) UNIQUE NOT NULL,"
                 "upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                 "status VARCHAR(50) DEFAULT 'uploaded')");
        
        txn.exec("CREATE TABLE IF NOT EXISTS reports ("
                 "id SERIAL PRIMARY KEY,"
                 "work_id INTEGER NOT NULL REFERENCES works(id) ON DELETE CASCADE,"
                 "analysis_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                 "plagiarism_found BOOLEAN DEFAULT FALSE,"
                 "similarity_percentage DECIMAL(5,2) DEFAULT 0.00,"
                 "matched_work_id INTEGER REFERENCES works(id),"
                 "matched_student_name VARCHAR(255),"
                 "report_data JSONB,"
                 "status VARCHAR(50) DEFAULT 'completed')");
        
        txn.commit();
        std::cout << "[DATABASE] Tables created/verified" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] Table creation: " << e.what() << std::endl;
        throw;
    }
}

int Database::save_work(const std::string& student_id,
                       const std::string& student_name,
                       const std::string& assignment_id,
                       const std::string& assignment_name,
                       const std::string& file_path,
                       const std::string& original_filename,
                       const std::string& file_hash) {
    try {
        pqxx::work txn(*conn);

        pqxx::result existing = txn.exec_params(
            "SELECT id, student_id FROM works WHERE file_hash = $1",
            file_hash
        );
        
        if (!existing.empty()) {
            int existing_id = existing[0]["id"].as<int>();
            std::string existing_student = existing[0]["student_id"].as<std::string>();
            
            if (existing_student != student_id) {
                txn.exec_params(
                    "UPDATE works SET status = 'duplicate_detected' WHERE id = $1",
                    existing_id
                );
            }
            
            txn.commit();
            return existing_id; 
        }

        pqxx::result result = txn.exec_params(
            "INSERT INTO works (student_id, student_name, assignment_id, assignment_name, "
            "file_path, original_filename, file_hash, status) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, 'uploaded') RETURNING id",
            student_id, student_name, assignment_id, assignment_name,
            file_path, original_filename, file_hash
        );
        
        int work_id = result[0]["id"].as<int>();
        txn.commit();
        
        std::cout << "[DATABASE] Work saved with ID: " << work_id << std::endl;
        return work_id;
        
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] save_work: " << e.what() << std::endl;
        throw;
    }
}

std::string Database::get_file_path(int work_id) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT file_path FROM works WHERE id = $1",
            work_id
        );
        
        if (result.empty()) {
            throw std::runtime_error("Work not found with ID: " + std::to_string(work_id));
        }
        
        return result[0]["file_path"].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] get_file_path: " << e.what() << std::endl;
        throw;
    }
}

bool Database::hash_exists(const std::string& file_hash) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT COUNT(*) as count FROM works WHERE file_hash = $1",
            file_hash
        );
        
        return result[0]["count"].as<int>() > 0;
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] hash_exists: " << e.what() << std::endl;
        throw;
    }
}

int Database::get_work_id_by_hash(const std::string& file_hash) {
    try {
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT id FROM works WHERE file_hash = $1",
            file_hash
        );
        
        if (result.empty()) {
            return -1;
        }
        
        return result[0]["id"].as<int>();
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] get_work_id_by_hash: " << e.what() << std::endl;
        return -1;
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
        std::cerr << "[DATABASE ERROR] update_work_status: " << e.what() << std::endl;
        throw;
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
        std::cout << "[DATABASE] Report saved for work ID: " << work_id << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] save_report: " << e.what() << std::endl;
        throw;
    }
}

std::string Database::get_report_json(int work_id) {
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
        std::cerr << "[DATABASE ERROR] get_report_json: " << e.what() << std::endl;
        return "{}";
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
        std::cerr << "[DATABASE ERROR] report_exists: " << e.what() << std::endl;
        return false;
    }
}

pqxx::result Database::get_works_by_assignment(const std::string& assignment_id) {
    try {
        pqxx::work txn(*conn);
        return txn.exec_params(
            "SELECT id, student_id, student_name, original_filename, "
            "upload_time, status FROM works WHERE assignment_id = $1 ORDER BY upload_time DESC",
            assignment_id
        );
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] get_works_by_assignment: " << e.what() << std::endl;
        throw;
    }
}

pqxx::result Database::get_all_reports() {
    try {
        pqxx::work txn(*conn);
        return txn.exec(
            "SELECT r.id, r.work_id, w.student_name, w.assignment_name, "
            "r.plagiarism_found, r.similarity_percentage, r.matched_student_name, "
            "r.analysis_time FROM reports r JOIN works w ON r.work_id = w.id "
            "ORDER BY r.analysis_time DESC"
        );
    } catch (const std::exception& e) {
        std::cerr << "[DATABASE ERROR] get_all_reports: " << e.what() << std::endl;
        throw;
    }
}