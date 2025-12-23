#pragma once
#include "domain.hpp"
#include <stdexcept>

class IdGenerator {
    int nextId = 1;
public:
    int newId() { return nextId++; }
};

class DomainFactory {
    IdGenerator idGen;
public:
    BankAccount createAccount(const std::string& name, double balance = 0.0) {
        return BankAccount{ idGen.newId(), name, balance };
    }
    
    Category createCategory(MoneyType type, const std::string& name) {
        return Category{ idGen.newId(), type, name };
    }
    
    Operation createOperation(MoneyType type, int bankAccountId, double amount,
                              const std::string& date, const std::string& description, int categoryId) {
        if (amount < 0) throw std::runtime_error("Amount cannot be negative");
        return Operation{ idGen.newId(), type, bankAccountId, amount, date, description, categoryId };
    }
};