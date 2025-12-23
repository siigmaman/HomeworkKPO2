#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <memory>
#include <string>
#include <pqxx/pqxx>

class Database {
public:
    Database(const std::string& host, 
             const std::string& port,
             const std::string& dbname,
             const std::string& user,
             const std::string& password);
    
    pqxx::transaction_base& begin_transaction();
    void commit() { if (transaction_) transaction_->commit(); }
    void rollback() { if (transaction_) transaction_->abort(); }
    
    pqxx::result query(const std::string& sql);
    pqxx::result query(pqxx::transaction_base& tx, const std::string& sql);
    
    void execute(const std::string& sql);
    void execute(pqxx::transaction_base& tx, const std::string& sql);
    
    template<typename... Args>
    pqxx::result query(const std::string& sql, Args&&... args);
    
    template<typename... Args>
    pqxx::result query(pqxx::transaction_base& tx, const std::string& sql, Args&&... args);
    
    template<typename... Args>
    void execute(const std::string& sql, Args&&... args);
    
    template<typename... Args>
    void execute(pqxx::transaction_base& tx, const std::string& sql, Args&&... args);
    
    void initialize_schema();
    
private:
    std::unique_ptr<pqxx::connection> conn_;
    std::unique_ptr<pqxx::work> transaction_;
};

#endif