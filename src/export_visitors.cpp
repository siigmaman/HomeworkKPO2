#include "export_visitors.hpp"
#include <sstream>

std::string CSVExportVisitor::visitAccount(const BankAccount& a) {
    std::ostringstream ss; ss << a.id << "," << a.name << "," << a.balance; return ss.str();
}
std::string CSVExportVisitor::visitCategory(const Category& c) {
    std::ostringstream ss; ss << c.id << "," << (c.type==MoneyType::Income?"Income":"Expense") << "," << c.name; return ss.str();
}
std::string CSVExportVisitor::visitOperation(const Operation& o) {
    std::ostringstream ss; ss << o.id << "," << (o.type==MoneyType::Income?"Income":"Expense") << "," << o.bank_account_id
       << "," << o.amount << "," << o.date << ",\"" << o.description << "\"," << o.category_id;
    return ss.str();
}

std::string JSONExportVisitor::visitAccount(const BankAccount& a) {
    std::ostringstream ss; ss << "{ \"id\": " << a.id << ", \"name\": \"" << a.name << "\", \"balance\": " << a.balance << " }"; return ss.str();
}
std::string JSONExportVisitor::visitCategory(const Category& c) {
    std::ostringstream ss; ss << "{ \"id\": " << c.id << ", \"type\": \"" << (c.type==MoneyType::Income?"Income":"Expense") << "\", \"name\": \"" << c.name << "\" }"; return ss.str();
}
std::string JSONExportVisitor::visitOperation(const Operation& o) {
    std::ostringstream ss; ss << "{ \"id\": " << o.id << ", \"type\": \"" << (o.type==MoneyType::Income?"Income":"Expense") << "\", \"bank_account_id\": " << o.bank_account_id
       << ", \"amount\": " << o.amount << ", \"date\": \"" << o.date << "\", \"description\": \"" << o.description << "\", \"category_id\": " << o.category_id << " }";
    return ss.str();
}