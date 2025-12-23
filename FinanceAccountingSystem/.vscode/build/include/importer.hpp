#pragma once
#include "repository.hpp"
#include "factory.hpp"
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

class BaseImporter {
protected:
    DomainFactory factory;
    
    virtual std::string readFile(const std::string& path) {
        std::ifstream in(path);
        if(!in) throw std::runtime_error("Cannot open file: " + path);
        std::ostringstream ss; 
        ss << in.rdbuf(); 
        return ss.str();
    }
    
    virtual void validateData(const std::string& data) {
        if (data.empty()) throw std::runtime_error("Empty file");
    }
    
    virtual void postImportFix(IRepository<BankAccount>&, IRepository<Category>&, IRepository<Operation>&) {}
    
public:
    virtual ~BaseImporter() = default;
    
    void importFromFile(const std::string& path, IRepository<BankAccount>& accR, IRepository<Category>& catR, IRepository<Operation>& opR) {
        auto data = readFile(path);
        validateData(data);
        parseAndLoad(data, accR, catR, opR);
        postImportFix(accR, catR, opR);
        std::cout << "Import completed successfully" << std::endl;
    }
    
protected:
    virtual void parseAndLoad(const std::string& raw, IRepository<BankAccount>& accR, IRepository<Category>& catR, IRepository<Operation>& opR) = 0;
};

class CSVImporter : public BaseImporter {
protected:
    void parseAndLoad(const std::string& raw, IRepository<BankAccount>& accR, IRepository<Category>& catR, IRepository<Operation>& opR) override {
        std::cout << "CSV Import would parse: " << raw.length() << " characters\n";
        if (accR.getAll().empty()) {
            auto acc = factory.createAccount("Imported CSV Account", 2000.0);
            accR.add(acc);
        }
    }
};

class JSONImporter : public BaseImporter {
protected:
    void parseAndLoad(const std::string& raw, IRepository<BankAccount>& accR, IRepository<Category>& catR, IRepository<Operation>& opR) override {
        std::cout << "JSON Import would parse: " << raw.length() << " characters\n";
        if (catR.getAll().empty()) {
            auto cat = factory.createCategory(MoneyType::Income, "Imported JSON Income");
            catR.add(cat);
        }
    }
};