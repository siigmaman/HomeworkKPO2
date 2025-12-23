#include "commands.hpp"
#include "facades.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using Clock = std::chrono::high_resolution_clock;

class CreateAccountCommand : public ICommand {
    AccountFacade& facade;
    std::string accountName;
    double start;
public:
    CreateAccountCommand(AccountFacade& f, std::string name_, double s)
        : facade(f), accountName(std::move(name_)), start(s) {}

    void execute() override { facade.create(accountName, start); }
    std::string name() const override { return "CreateAccount"; }
};

class CreateCategoryCommand : public ICommand {
    CategoryFacade& facade;
    MoneyType type;
    std::string categoryName;
public:
    CreateCategoryCommand(CategoryFacade& f, MoneyType t, std::string name_)
        : facade(f), type(t), categoryName(std::move(name_)) {}

    void execute() override { facade.create(type, categoryName); }
    std::string name() const override { return "CreateCategory"; }
};

class CreateOperationCommand : public ICommand {
    OperationFacade& facade;
    MoneyType type;
    int accId;
    double amount;
    std::string date;
    std::string description;
    int catId;
public:
    CreateOperationCommand(OperationFacade& f, MoneyType t, int accId_, double amount_,
                           std::string date_, std::string desc_, int catId_)
        : facade(f), type(t), accId(accId_), amount(amount_),
          date(std::move(date_)), description(std::move(desc_)), catId(catId_) {}

    void execute() override { facade.create(type, accId, amount, date, description, catId); }
    std::string name() const override { return "CreateOperation"; }
};

class TimedCommand : public ICommand {
    std::unique_ptr<ICommand> inner;
    std::function<void(const std::string&, double)> logger;
public:
    TimedCommand(std::unique_ptr<ICommand> c, std::function<void(const std::string&, double)> log)
        : inner(std::move(c)), logger(std::move(log)) {}

    void execute() override {
        auto t1 = Clock::now();
        inner->execute();
        auto t2 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        logger(inner->name(), ms);
    }

    std::string name() const override {
        return "Timed(" + inner->name() + ")";
    }
};