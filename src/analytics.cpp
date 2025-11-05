#include "analytics.hpp"
double AnalyticsFacade::incomeMinusExpense(const std::string& fromDate, const std::string& toDate) {
    double inc=0, exp=0;
    for(auto &op : opRepo.getAll()) {
        if(op.date >= fromDate && op.date <= toDate) {
            if(op.type==MoneyType::Income) inc += op.amount;
            else exp += op.amount;
        }
    }
    return inc - exp;
}
std::map<int,double> AnalyticsFacade::sumByCategory(const std::string& fromDate, const std::string& toDate) {
    std::map<int,double> res;
    for(auto &op: opRepo.getAll()) {
        if(op.date >= fromDate && op.date <= toDate) {
            res[op.category_id] += (op.type==MoneyType::Income ? op.amount : -op.amount);
        }
    }
    return res;
}