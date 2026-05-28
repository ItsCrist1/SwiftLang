#ifndef SHELLANG_SCOPEUNIT_H
#define SHELLANG_SCOPEUNIT_H

#include <unordered_map>
#include <string>
#include <memory>

#define VMAP std::unordered_map<std::string,T>

template<typename T>
struct ScopeUnit {
    explicit ScopeUnit(std::shared_ptr<VMAP> Globals) : Globals(std::move(Globals)) {}

    [[nodiscard]] const T& Get(const std::string& key) const {
        if(const auto& res = Locals.find(key); res != Locals.end())
            return res->second;
        if(const auto& res = Globals->find(key); res != Globals->end())
            return res->second;

        static constexpr T empty;
        return empty;
    }

    void Set(const std::string& key, const T& value) {
        if(const auto res=Locals.find(key);
            res != Locals.end()) {
            res->second = value;
            return;
        }

        if(const auto res=Globals->find(key);
            res != Globals->end()) {
            res->second = value;
            return;
        }

        Locals[key] = value;
    }

    [[nodiscard]] T& GetDynamic(const std::string& key) {
        if(auto res = Locals.find(key); res != Locals.end())
            return res->second;
        if(auto res = Globals->find(key); res != Globals->end())
            return res->second;

        return Locals[key];
    }

private:
    std::shared_ptr<VMAP> Globals;
    VMAP Locals;
};

#undef VMAP

#endif