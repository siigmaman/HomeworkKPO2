#pragma once
#include <string>
#include <optional>
#include <vector>
#include <map>

enum class MoneyType { Income, Expense };

struct BankAccount {
    int id;
    std::string name;
    double balance;
};

struct Category {
    int id;
    MoneyType type;
    std::string name;
};

struct Operation {
    int id;
    MoneyType type;
    int bank_account_id;
    double amount;
    std::string date; 
    std::string description;
    int category_id;
};