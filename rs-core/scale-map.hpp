#pragma once

#include "rs-core/common.hpp"
#include "rs-core/float.hpp"
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <map>
#include <type_traits>
#include <utility>

namespace RS {

    template <typename X, typename Y = X>
    class ScaleMap {
    private:
        static_assert(std::is_floating_point<X>::value);
        struct init_type {
            X x0;
            Y y1, y2, y3;
            init_type(X x, Y y) noexcept: x0(x), y1(y), y2(y), y3(y) {}
            init_type(X x, Y yl, Y yr) noexcept: x0(x), y1(yl), y2(mid(yl, yr)), y3(yr) {}
            init_type(X x, Y yl, Y y, Y yr) noexcept: x0(x), y1(yl), y2(y), y3(yr) {}
        };
        struct y_values { Y left, mid, right; };
        using map_type = std::map<X, y_values>;
        map_type map;
        static inline Y mid(Y y1, Y y2) noexcept { return interpolate(X(0), y1, X(2), y2, X(1)); }
    public:
        using key_type = X;
        using mapped_type = Y;
        ScaleMap() = default;
        ScaleMap(std::initializer_list<init_type> list);
        Y operator()(X x) const;
        void clear() noexcept { map.clear(); }
        bool empty() const noexcept { return map.empty(); }
        void erase(X x) noexcept { map.erase(x); }
        void erase(X x1, X x2) noexcept;
        void insert(X x, Y y) { map[x] = {y, y, y}; }
        void insert(X x, Y yl, Y yr) { map[x] = {yl, mid(yl, yr), yr}; }
        void insert(X x, Y yl, Y y, Y yr) { map[x] = {yl, y, yr}; }
        X min() const noexcept { return map.empty() ? X(0) : map.begin()->first; }
        X max() const noexcept { return map.empty() ? X(0) : std::prev(map.end())->first; }
        template <typename T> void scale_x(T a, X b = X(0));
        template <typename T> void scale_y(T a, Y b = Y());
    };

    template <typename X, typename Y>
    ScaleMap<X, Y>::ScaleMap(std::initializer_list<init_type> list) {
        for (auto& in: list)
            map.insert({in.x0, {in.y1, in.y2, in.y3}});
    }

    template <typename X, typename Y>
    Y ScaleMap<X, Y>::operator()(X x) const {
        if (map.empty())
            return Y();
        auto i = map.lower_bound(x);
        if (i == map.end())
            return std::prev(i)->second.right;
        if (i->first == x)
            return i->second.mid;
        if (i == map.begin())
            return i->second.left;
        auto j = std::prev(i);
        return interpolate(j->first, j->second.right, i->first, i->second.left, x);
    }

    template <typename X, typename Y>
    void ScaleMap<X, Y>::erase(X x1, X x2) noexcept {
        if (x1 > x2)
            std::swap(x1, x2);
        auto i = map.lower_bound(x1);
        auto j = map.upper_bound(x2);
        map.erase(i, j);
    }

    template <typename X, typename Y>
    template <typename T>
    void ScaleMap<X, Y>::scale_x(T a, X b) {
        static_assert(std::is_arithmetic<T>::value);
        if (map.empty()) {
            // do nothing
        } else if (a == T(0)) {
            Y y1 = map.begin()->second.left;
            Y y2;
            Y y3 = std::prev(map.end())->second.right;
            if (map.size() == 1)
                y2 = map.begin()->second.mid;
            else
                y2 = mid(y1, y3);
            map.clear();
            insert(b, y1, y2, y3);
        } else {
            map_type new_map;
            for (auto& xy: map) {
                X x = a * xy.first + b;
                if (a < T(0))
                    std::swap(xy.second.left, xy.second.right);
                new_map[x] = xy.second;
            }
            map = std::move(new_map);
        }
    }

    template <typename X, typename Y>
    template <typename T>
    void ScaleMap<X, Y>::scale_y(T a, Y b) {
        static_assert(std::is_arithmetic<T>::value);
        for (auto& xy: map) {
            xy.second.left = a * xy.second.left + b;
            xy.second.mid = a * xy.second.mid + b;
            xy.second.right = a * xy.second.right + b;
        }
    }

}
