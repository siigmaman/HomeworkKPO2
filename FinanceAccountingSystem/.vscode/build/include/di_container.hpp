#pragma once
#include <functional>
#include <unordered_map>
#include <string>

class ServiceContainer {
    std::unordered_map<std::string, std::function<void*()>> creators;
    std::unordered_map<std::string, void*> instances;
public:
    template<typename T>
    void registerInstance(const std::string& key, T* instance) {
        instances[key] = instance;
    }
    template<typename T>
    T* resolve(const std::string& key) {
        auto it = instances.find(key);
        if(it!=instances.end()) return static_cast<T*>(it->second);
        auto itc = creators.find(key);
        if(itc!=creators.end()) {
            T* created = static_cast<T*>(itc->second());
            instances[key] = created;
            return created;
        }
        return nullptr;
    }
    template<typename T>
    void registerFactory(const std::string& key, std::function<T*()> f) {
        creators[key] = [f](){ return (void*)f(); };
    }
};