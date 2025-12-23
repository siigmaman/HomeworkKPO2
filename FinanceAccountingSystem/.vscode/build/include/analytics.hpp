#pragma once
#include "repository.hpp"
#include <map>

class AnalyticsFacade {
    IRepository<Operation>& opRepo;
    IRepository<Category>& catRepo;
public:
    AnalyticsFacade(IRepository<Operation>& o, IRepository<Category>& c) : opRepo(o), catRepo(c) {}
    double incomeMinusExpense(const std::string& fromDate, const std::string& toDate);
    std::map<int,double> sumByCategory(const std::string& fromDate, const std::string& toDate);
};