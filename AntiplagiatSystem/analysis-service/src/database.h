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

    struct WorkInfo {
        int id;
        std::string student_id;
        std::string student_name;
        std::string assignment_id;
        std::string file_path;
        std::string file_hash;
        std::string status;
    };
    
    WorkInfo get_work_info(int work_id);
    std::string get_work_file_path(int work_id);
    std::string get_work_file_hash(int work_id);
    std::string get_student_id_by_hash(const std::string& file_hash);

    struct SimilarWork {
        int id;
        std::string student_id;
        std::string student_name;
        std::string file_hash;
    };
    
    std::vector<SimilarWork> find_similar_works(const std::string& file_hash, 
                                                const std::string& exclude_student_id);

    void save_report(int work_id,
                     bool plagiarism_found,
                     double similarity_percentage,
                     int matched_work_id,
                     const std::string& matched_student_name,
                     const std::string& report_data);
    
    bool report_exists(int work_id);
    std::string get_report(int work_id);

    void update_work_status(int work_id, const std::string& status);
    
private:
    void create_tables();
};