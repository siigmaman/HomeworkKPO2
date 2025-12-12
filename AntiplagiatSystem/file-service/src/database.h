#pragma once
#include <string>
#include <memory>
#include <pqxx/pqxx>

class Database {
private:
    std::unique_ptr<pqxx::connection> conn;
    
public:
    Database(const std::string& connection_string);
    ~Database();

    bool is_connected() const;

    int save_work(const std::string& student_id,
                  const std::string& student_name,
                  const std::string& assignment_id,
                  const std::string& assignment_name,
                  const std::string& file_path,
                  const std::string& original_filename,
                  const std::string& file_hash);
    
    std::string get_file_path(int work_id);
    std::string get_file_hash(int work_id);
    std::string get_student_id(int work_id);
    bool hash_exists(const std::string& file_hash);
    int get_work_id_by_hash(const std::string& file_hash);
    void update_work_status(int work_id, const std::string& status);

    void save_report(int work_id,
                     bool plagiarism_found,
                     double similarity_percentage,
                     int matched_work_id,
                     const std::string& matched_student_name,
                     const std::string& report_data);
    
    std::string get_report_json(int work_id);
    bool report_exists(int work_id);

    pqxx::result get_works_by_assignment(const std::string& assignment_id);
    pqxx::result get_all_reports();
    
private:
    void create_tables();
};