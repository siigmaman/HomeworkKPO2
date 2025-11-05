#pragma once
#include "domain.hpp"
#include "repository.hpp"
#include <stdexcept>

class AccountFacade {
    IRepository<BankAccount>& repo;
public:
    AccountFacade(IRepository<BankAccount>& r): repo(r) {}
    void create(const std::string& name, double startBalance);
    void remove(int id) { repo.remove(id); }
    std::optional<BankAccount> get(int id) { return repo.get(id); }
    std::vector<BankAccount> all() { return repo.getAll(); }
    void updateBalance(int id, double newBalance);
};

class CategoryFacade {
    IRepository<Category>& repo;
public:
    CategoryFacade(IRepository<Category>& r): repo(r) {}
    void create(MoneyType t, const std::string& name);
    void remove(int id) { repo.remove(id); }
    std::vector<Category> all() { return repo.getAll(); }
    std::optional<Category> get(int id) { return repo.get(id); }
};

class OperationFacade {
    IRepository<Operation>& repo;
    IRepository<BankAccount>& accountRepo;
public:
    OperationFacade(IRepository<Operation>& r, IRepository<BankAccount>& accR) : repo(r), accountRepo(accR) {}
    void create(MoneyType type, int bankAccountId, double amount, const std::string& date, const std::string& description, int categoryId);
    void remove(int id);
    std::vector<Operation> all() { return repo.getAll(); }
    std::optional<Operation> get(int id) { return repo.get(id); }
};