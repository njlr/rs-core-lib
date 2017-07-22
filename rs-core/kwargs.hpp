#pragma once

#include "rs-core/common.hpp"
#include <type_traits>

namespace RS {

    // Keyword arguments

    namespace RS_Detail {

        template <typename K, typename V, bool = std::is_convertible<K, V>::value> struct Kwcopy;
        template <typename K, typename V> struct Kwcopy<K, V, true> { void operator()(const K& a, V& p) const { p = V(a); } };
        template <typename K, typename V> struct Kwcopy<K, V, false> { void operator()(const K&, V&) const {} };

        template <typename K> struct Kwparam {
            const void* key;
            K val;
        };

    }

    template <typename K> struct Kwarg {
        constexpr Kwarg() {}
        template <typename A> RS_Detail::Kwparam<K> operator=(const A& a) const { return {this, K(a)}; }
    };

    template <typename K, typename V, typename K2, typename... Args>
    bool kwget(const Kwarg<K>& k, V& v, const RS_Detail::Kwparam<K2>& p, const Args&... args) {
        if (&k != p.key)
            return kwget(k, v, args...);
        RS_Detail::Kwcopy<K2, V>()(p.val, v);
        return true;
    }

    template <typename K, typename V, typename... Args>
    bool kwget(const Kwarg<K>& k, V& v, const Kwarg<bool>& p, const Args&... args) {
        return kwget(k, v, p = true, args...);
    }

    template <typename K, typename V> bool kwget(const Kwarg<K>&, V&) { return false; }

}
