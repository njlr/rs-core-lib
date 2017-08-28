#pragma once

#include "rs-core/common.hpp"
#include <utility>
#include <vector>

namespace RS {

    template <typename T>
    class Stack {
    public:
        using const_iterator = typename std::vector<T>::const_iterator;
        using iterator = typename std::vector<T>::iterator;
        using value_type = T;
        Stack() = default;
        ~Stack() noexcept { clear(); }
        Stack(const Stack&) = delete;
        Stack(Stack&& s) = default;
        Stack& operator=(const Stack&) = delete;
        Stack& operator=(Stack&& s) { if (&s != this) { clear(); con = std::move(s.con); } return *this; }
        iterator begin() noexcept { return con.begin(); }
        const_iterator begin() const noexcept { return con.begin(); }
        iterator end() noexcept { return con.end(); }
        const_iterator end() const noexcept { return con.end(); }
        void clear() noexcept { while (! con.empty()) con.pop_back(); }
        template <typename... Args> void emplace(Args&&... args) { con.emplace_back(std::forward<Args>(args)...); }
        bool empty() const noexcept { return con.empty(); }
        void pop() noexcept { con.pop_back(); }
        void push(const T& t) { con.push_back(t); }
        void push(T&& t) { con.push_back(std::move(t)); }
        size_t size() const noexcept { return con.size(); }
        T& top() noexcept { return con.back(); }
        const T& top() const noexcept { return con.back(); }
    private:
        std::vector<T> con;
    };

}
