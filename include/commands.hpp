#pragma once
#include <string>
#include <functional>
#include <memory>
#include "facades.hpp"

struct ICommand {
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual std::string name() const = 0;
};

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