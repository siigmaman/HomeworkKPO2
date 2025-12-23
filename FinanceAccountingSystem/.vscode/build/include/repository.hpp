#pragma once
#include "domain.hpp"
#include <vector>
#include <optional>
#include <unordered_map>
#include <algorithm>
#include <iostream>

template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual void add(const T& obj) = 0;
    virtual void update(const T& obj) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<T> get(int id) = 0;
    virtual std::vector<T> getAll() = 0;
};

template<typename T>
class InMemoryRepository : public IRepository<T> {
protected:
    std::unordered_map<int, T> storage;
public:
    void add(const T& obj) override { 
        storage[obj.id] = obj; 
        std::cout << "Added object with id " << obj.id << std::endl;
    }
    
    void update(const T& obj) override { 
        storage[obj.id] = obj; 
    }
    
    void remove(int id) override { 
        storage.erase(id); 
    }
    
    std::optional<T> get(int id) override {
        auto it = storage.find(id);
        if (it == storage.end()) return std::nullopt;
        return it->second;
    }
    
    std::vector<T> getAll() override {
        std::vector<T> res; 
        res.reserve(storage.size());
        for (auto &p: storage) res.push_back(p.second);
        std::sort(res.begin(), res.end(), [](const T&a,const T&b){return a.id<b.id;});
        return res;
    }
};