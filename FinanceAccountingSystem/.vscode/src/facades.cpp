#include "facades.hpp"
#include "factory.hpp"

void AccountFacade::create(const std::string& name, double startBalance) {
    static DomainFactory f;
    auto acc = f.createAccount(name, startBalance);
    repo.add(acc);
}

void AccountFacade::updateBalance(int id, double newBalance) {
    auto o = repo.get(id);
    if (!o) throw std::runtime_error("Account not found");
    auto acc = *o; acc.balance = newBalance;
    repo.update(acc);
}

void CategoryFacade::create(MoneyType t, const std::string& name) {
    static DomainFactory f;
    auto c = f.createCategory(t, name);
    repo.add(c);
}

void OperationFacade::create(MoneyType type, int bankAccountId, double amount, const std::string& date, const std::string& description, int categoryId) {
    static DomainFactory f;
    auto accOpt = accountRepo.get(bankAccountId);
    if(!accOpt) throw std::runtime_error("Bank account not found");
    Operation op = f.createOperation(type, bankAccountId, amount, date, description, categoryId);
    repo.add(op);
    auto acc = *accOpt;
    if (type == MoneyType::Income) acc.balance += amount;
    else acc.balance -= amount;
    accountRepo.update(acc);
}

void OperationFacade::remove(int id) {
    auto opOpt = repo.get(id);
    if(!opOpt) throw std::runtime_error("Operation not found");
    auto op = *opOpt;
    auto accOpt = accountRepo.get(op.bank_account_id);
    if(accOpt) {
        auto acc = *accOpt;
        if(op.type == MoneyType::Income) acc.balance -= op.amount;
        else acc.balance += op.amount;
        accountRepo.update(acc);
    }
    repo.remove(id);
}