#include "repository.hpp"
#include <unordered_map>
#include <algorithm>
#include <iostream>

template<typename T>
class InMemoryRepository : public IRepository<T> {
protected:
    std::unordered_map<int, T> storage;
public:
    void add(const T& obj) override { 
        storage[obj.id] = obj; 
        std::cout << "Added " << typeid(T).name() << " with id " << obj.id << std::endl;
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

template class InMemoryRepository<BankAccount>;
template class InMemoryRepository<Category>;
template class InMemoryRepository<Operation>;

template<typename T>
class SimpleCachedRepository : public IRepository<T> {
    std::unordered_map<int, T> cache;
public:
    void add(const T& obj) override { 
        cache[obj.id] = obj; 
    }
    
    void update(const T& obj) override { 
        cache[obj.id] = obj; 
    }
    
    void remove(int id) override { 
        cache.erase(id); 
    }
    
    std::optional<T> get(int id) override {
        auto it = cache.find(id);
        if (it == cache.end()) return std::nullopt;
        return it->second;
    }
    
    std::vector<T> getAll() override {
        std::vector<T> res;
        for (const auto& pair : cache) {
            res.push_back(pair.second);
        }
        std::sort(res.begin(), res.end(), [](const T& a, const T& b) { return a.id < b.id; });
        return res;
    }
};

template class SimpleCachedRepository<BankAccount>;
template class SimpleCachedRepository<Category>;
template class SimpleCachedRepository<Operation>;