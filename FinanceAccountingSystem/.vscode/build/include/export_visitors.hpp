#pragma once
#include "domain.hpp"
#include <string>

struct ExportVisitor {
    virtual ~ExportVisitor() = default;
    virtual std::string visitAccount(const BankAccount& a) = 0;
    virtual std::string visitCategory(const Category& c) = 0;
    virtual std::string visitOperation(const Operation& o) = 0;
};

struct CSVExportVisitor : public ExportVisitor {
    std::string visitAccount(const BankAccount& a) override;
    std::string visitCategory(const Category& c) override;
    std::string visitOperation(const Operation& o) override;
};

struct JSONExportVisitor : public ExportVisitor {
    std::string visitAccount(const BankAccount& a) override;
    std::string visitCategory(const Category& c) override;
    std::string visitOperation(const Operation& o) override;
};