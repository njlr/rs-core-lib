#pragma once

#include "rs-core/common.hpp"
#include "rs-core/float.hpp"
#include "rs-core/string.hpp"
#include "rs-core/vector.hpp"
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RS {

    // ArrayMap

    namespace RS_Detail {

        template <typename K, typename T>
        struct ArrayMapTraits {
            using value_type = std::pair<K, T>;
            using mapped_ref = T&;
            using mapped_cref = const T&;
            static const K& key(const value_type& v) noexcept { return v.first; }
            static bool match(const K& k, const value_type& v) noexcept { return k == v.first; }
        };

        template <typename K>
        struct ArrayMapTraits<K, void> {
            using value_type = K;
            using mapped_ref = void*;
            using mapped_cref = const void*;
            static const K& key(const K& v) noexcept { return v; }
            static bool match(const K& k, const K& v) noexcept { return k == v; }
        };

    }

    template <typename K, typename T>
    class ArrayMap {
    private:
        using traits = RS_Detail::ArrayMapTraits<K, T>;
        using array_type = std::vector<typename traits::value_type>;
        array_type array;
    public:
        using const_iterator = typename array_type::const_iterator;
        using iterator = typename array_type::iterator;
        using key_type = K;
        using mapped_type = T;
        using value_type = typename traits::value_type;
        ArrayMap() = default;
        ArrayMap(std::initializer_list<value_type> list): array(list) {}
        typename traits::mapped_ref operator[](const K& k); // map only
        iterator begin() noexcept { return array.begin(); }
        const_iterator begin() const noexcept { return array.begin(); }
        iterator end() noexcept { return array.end(); }
        const_iterator end() const noexcept { return array.end(); }
        size_t capacity() const noexcept { return array.capacity(); }
        void clear() noexcept { array.clear(); }
        bool empty() const noexcept { return array.empty(); }
        bool erase(const K& k) noexcept;
        void erase(const_iterator i) noexcept { array.erase(i); }
        iterator find(const K& k) noexcept { return std::find_if(begin(), end(), [&] (const value_type& v) { return traits::match(k, v); }); }
        const_iterator find(const K& k) const noexcept { return std::find_if(begin(), end(), [&] (const value_type& v) { return traits::match(k, v); }); }
        bool has(const K& k) const noexcept { return find(k) != end(); }
        std::pair<iterator, bool> insert(const value_type& v);
        void reserve(size_t n) { array.reserve(n); }
        size_t size() const noexcept { return array.size(); }
    };

    template <typename K> using ArraySet = ArrayMap<K, void>;

    template <typename K, typename T>
    typename ArrayMap<K, T>::traits::mapped_ref ArrayMap<K, T>::operator[](const K& k) {
        auto i = find(k);
        if (i == end()) {
            array.push_back({k, {}});
            i = std::prev(end());
        }
        return i->second;
    }

    template <typename K, typename T>
    bool ArrayMap<K, T>::erase(const K& k) noexcept {
        auto i = find(k);
        if (i == end())
            return false;
        array.erase(i);
        return true;
    }

    template <typename K, typename T>
    std::pair<typename ArrayMap<K, T>::iterator, bool> ArrayMap<K, T>::insert(const value_type& v) {
        auto i = find(traits::key(v));
        if (i != end())
            return {i, false};
        array.push_back(v);
        return {std::prev(end()), true};
    }

    // Blob

    class Blob:
    public LessThanComparable<Blob> {
    public:
        Blob() = default;
        explicit Blob(size_t n) { init_size(n); }
        Blob(size_t n, uint8_t x) { init_size(n); memset(ptr, x, len); }
        Blob(void* p, size_t n): Blob(p, n, &std::free) {}
        template <typename F> Blob(void* p, size_t n, F f) { if (p && n) {ptr = p; len = n; del = f; } }
        ~Blob() noexcept { if (ptr && del) try { del(ptr); } catch (...) {} }
        Blob(const Blob& b) { init_copy(b.data(), b.size()); }
        Blob(Blob&& b) noexcept: ptr(b.ptr), len(b.len) { del.swap(b.del); }
        Blob& operator=(const Blob& b) { copy(b.data(), b.size()); return *this; }
        Blob& operator=(Blob&& b) noexcept { Blob b2(std::move(b)); swap(b2); return *this; }
        void* data() noexcept { return ptr; }
        const void* data() const noexcept { return ptr; }
        uint8_t* bdata() noexcept { return static_cast<uint8_t*>(ptr); }
        const uint8_t* bdata() const noexcept { return static_cast<const uint8_t*>(ptr); }
        char* cdata() noexcept { return static_cast<char*>(ptr); }
        const char* cdata() const noexcept { return static_cast<const char*>(ptr); }
        Irange<uint8_t*> bytes() noexcept { return {bdata(), bdata() + len}; }
        Irange<const uint8_t*> bytes() const noexcept { return {bdata(), bdata() + len}; }
        Irange<char*> chars() noexcept { return {cdata(), cdata() + len}; }
        Irange<const char*> chars() const noexcept { return {cdata(), cdata() + len}; }
        void clear() noexcept { Blob b; swap(b); }
        void copy(const void* p, size_t n) { Blob b; b.init_copy(p, n); swap(b); }
        bool empty() const noexcept { return len == 0; }
        void fill(uint8_t x) noexcept { memset(ptr, x, len); }
        size_t hash() const noexcept { return djb2a(ptr, len); }
        U8string hex(size_t block = 0) const { return hexdump(ptr, len, block); }
        void reset(size_t n) { Blob b(n); swap(b); }
        void reset(size_t n, uint8_t x) { Blob b(n, x); swap(b); }
        void reset(void* p, size_t n) { Blob b(p, n); swap(b); }
        template <typename F> void reset(void* p, size_t n, F f) { Blob b(p, n, f); swap(b); }
        size_t size() const noexcept { return len; }
        std::string str() const { return empty() ? std::string() : std::string(cdata(), len); }
        void swap(Blob& b) noexcept { std::swap(ptr, b.ptr); std::swap(len, b.len); del.swap(b.del); }
    private:
        void* ptr = nullptr;
        size_t len = 0;
        std::function<void(void*)> del;
        void init_copy(const void* p, size_t n) {
            if (p && n) {
                init_size(n);
                memcpy(ptr, p, n);
            }
        }
        void init_size(size_t n) {
            if (n) {
                ptr = std::malloc(n);
                if (! ptr)
                    throw std::bad_alloc();
                len = n;
                del = &std::free;
            }
        }
    };

    inline bool operator==(const Blob& lhs, const Blob& rhs) noexcept {
        return lhs.size() == rhs.size() && memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
    }

    inline bool operator<(const Blob& lhs, const Blob& rhs) noexcept {
        auto cmp = memcmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
        return cmp < 0 || (cmp == 0 && lhs.size() < rhs.size());
    }

    inline void swap(Blob& b1, Blob& b2) noexcept { b1.swap(b2); }

    // Grid

    template <typename T, size_t N>
    class Grid:
    public EqualityComparable<Grid<T, N>> {
    private:
        template <bool IsConst>
        class basic_iterator:
        public ForwardIterator<basic_iterator<IsConst>, std::conditional_t<IsConst, const T, T>> {
        private:
            using grid_type = std::conditional_t<IsConst, const Grid, Grid>;
            using target_type = std::conditional_t<IsConst, const T, T>;
        public:
            basic_iterator() = default;
            basic_iterator(const basic_iterator<false>& i): grid(i.grid), ofs(i.ofs) {}
            target_type& operator*() const noexcept { return grid->data[ofs]; }
            basic_iterator& operator++() { ++ofs; return *this; }
            bool operator==(const basic_iterator& rhs) const noexcept { return ofs == rhs.ofs; }
            void move(size_t axis, ptrdiff_t delta) noexcept { ofs += delta * grid->scale[axis]; }
            typename Grid::index_type pos() const { return grid->get_index(ofs); }
        private:
            friend class Grid;
            friend class basic_iterator<true>;
            grid_type* grid;
            ptrdiff_t ofs; // Index into grid->data
            basic_iterator(grid_type& g, ptrdiff_t o): grid(&g), ofs(o) {}
        };
    public:
        using const_iterator = basic_iterator<true>;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
        using index_type = Vector<ptrdiff_t, N>;
        using iterator = basic_iterator<false>;
        using reference = T&;
        using size_type = size_t;
        using value_type = T;
        static constexpr size_t dim = N;
        Grid(): extent(), scale(), data() { zero_coords(); }
        explicit Grid(const index_type& shape): extent(shape), scale(make_scale(extent)), data(size()) {}
        Grid(const index_type& shape, const T& t): extent(shape), scale(make_scale(extent)), data(size(), t) {}
        template <typename... Args> explicit Grid(Args... shape): extent{shape...}, scale(make_scale(extent)), data(size()) {}
        T& operator[](const index_type& pos) noexcept { return data[get_offset(pos)]; }
        const T& operator[](const index_type& pos) const noexcept { return data[get_offset(pos)]; }
        template <typename... Args> T& operator()(Args... pos) noexcept { return data[get_offset(index_type{pos...})]; }
        template <typename... Args> const T& operator()(Args... pos) const noexcept { return data[get_offset(index_type{pos...})]; }
        T& at(const index_type& pos) { return data[get_offset_checked(pos)]; }
        const T& at(const index_type& pos) const { return data[get_offset_checked(pos)]; }
        template <typename... Args> T& at(Args... pos) { return data[get_offset_checked(index_type{pos...})]; }
        template <typename... Args> const T& at(Args... pos) const { return data[get_offset_checked(index_type{pos...})]; }
        iterator begin() noexcept { return {*this, 0}; }
        const_iterator begin() const noexcept { return {*this, 0}; }
        void clear() noexcept { zero_coords(); data.clear(); }
        bool contains(const index_type& pos) const noexcept;
        template <typename... Args> bool contains(Args... pos) const noexcept { return contains(index_type{pos...}); }
        bool empty() const noexcept { return data.empty(); }
        iterator end() noexcept { return {*this, ptrdiff_t(size())}; }
        const_iterator end() const noexcept { return {*this, ptrdiff_t(size())}; }
        void fill(const T& t) { std::fill(begin(), end(), t); }
        iterator locate(const index_type& pos) noexcept { return {*this, get_offset(pos)}; }
        const_iterator locate(const index_type& pos) const noexcept { return {*this, get_offset(pos)}; }
        template <typename... Args> iterator locate(Args... pos) noexcept { return {*this, get_offset(index_type{pos...})}; }
        template <typename... Args> const_iterator locate(Args... pos) const noexcept { return {*this, get_offset(index_type{pos...})}; }
        void reset(const index_type& shape, const T& t = T()) { Grid g(shape, t); *this = std::move(g); }
        template <typename... Args> void reset(Args... shape) { Grid g(shape...); *this = std::move(g); }
        void reshape(const index_type& shape, const T& t = T());
        template <typename... Args> void reshape(Args... shape) { reshape(index_type{shape...}); }
        index_type shape() const noexcept { return extent; }
        size_t size() const noexcept { return extent[0] * scale[0]; }
    private:
        static_assert(N > 0);
        using data_type = std::vector<T>;
        index_type extent;
        index_type scale;
        data_type data;
        index_type get_index(ptrdiff_t ofs) const noexcept;
        constexpr ptrdiff_t get_offset(const index_type& pos) const noexcept { return std::inner_product(pos.begin(), pos.end(), scale.begin(), size_t(0)); }
        ptrdiff_t get_offset_checked(const index_type& pos) const;
        void zero_coords() noexcept;
        static index_type make_scale(const index_type& extent) noexcept;
        static void reshape_copy(const Grid& src, Grid& dst, index_type index, ptrdiff_t current);
    };

    template <typename T, size_t N>
    bool Grid<T, N>::contains(const index_type& pos) const noexcept {
        for (size_t i = 0; i < N; ++i)
            if (pos[i] < 0 || pos[i] >= extent[i])
                return false;
        return true;
    }

    template <typename T, size_t N>
    void Grid<T, N>::reshape(const index_type& shape, const T& t) {
        Grid g(shape, t);
        reshape_copy(*this, g, index_type(), 0);
        *this = std::move(g);
    }

    template <typename T, size_t N>
    typename Grid<T, N>::index_type Grid<T, N>::get_index(ptrdiff_t ofs) const noexcept {
        auto x = ofs;
        index_type pos;
        for (size_t i = 0; i < N; ++i) {
            pos[i] = x / scale[i];
            x %= scale[i];
        }
        return pos;
    }

    template <typename T, size_t N>
    ptrdiff_t Grid<T, N>::get_offset_checked(const index_type& pos) const {
        for (size_t i = 0; i < N; ++i)
            if (pos[i] < 0 || pos[i] >= extent[i])
                throw std::out_of_range("Grid index out of range");
        return get_offset(pos);
    }

    template <typename T, size_t N>
    void Grid<T, N>::zero_coords() noexcept {
        std::fill(extent.begin(), extent.end(), 0);
        std::fill(scale.begin(), scale.end(), 0);
    }

    template <typename T, size_t N>
    typename Grid<T, N>::index_type Grid<T, N>::make_scale(const index_type& extent) noexcept {
        index_type scale;
        scale[N - 1] = 1;
        for (size_t i = N - 1; i > 0; --i)
            scale[i - 1] = scale[i] * extent[i];
        return scale;
    }

    template <typename T, size_t N>
    void Grid<T, N>::reshape_copy(const Grid& src, Grid& dst, index_type index, ptrdiff_t current) {
        ptrdiff_t n_common = std::min(src.shape()[current], dst.shape()[current]);
        ptrdiff_t& iref = index[current];
        if (current == N - 1) {
            iref = 0;
            std::copy_n(&src[index], n_common, &dst[index]);
        } else {
            for (iref = 0; iref < n_common; ++iref)
                reshape_copy(src, dst, index, current + 1);
        }
    }

    template <typename T, size_t N>
    bool operator==(const Grid<T, N>& lhs, const Grid<T, N>& rhs) noexcept {
        return lhs.shape() == rhs.shape() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    // IndexTable

    RS_ENUM_CLASS(IndexMode, int, 1, unique, duplicate);

    template <typename T> class IndexTable;
    template <typename K, typename T, IndexMode M = IndexMode::unique> class Index;

    class IndexCollision:
    public std::runtime_error {
    public:
        IndexCollision(): std::runtime_error("Index collision") {}
    };

    namespace RS_Detail {

        template <typename K, typename T, IndexMode M> struct IndexMapTraits;

        template <typename K, typename T>
        struct IndexMapTraits<K, T, IndexMode::unique> {
            using compare_function = std::function<bool(const K&, const K&)>;
            using map_type = std::map<K, T, compare_function>;
            using value_type = typename map_type::value_type;
            static void insert(const K& k, const T& value, map_type& map) {
                if (! map.insert(value_type(k, value)).second)
                    throw IndexCollision();
            }
        };

        template <typename K, typename T>
        struct IndexMapTraits<K, T, IndexMode::duplicate> {
            using compare_function = std::function<bool(const K&, const K&)>;
            using map_type = std::multimap<K, T, compare_function>;
            using value_type = typename map_type::value_type;
            static void insert(const K& k, const T& value, map_type& map) {
                map.insert(value_type(k, value));
            }
        };

        template <typename T>
        class IndexRefBase {
        public:
            RS_NO_COPY_MOVE(IndexRefBase);
            using list_type = std::list<T>;
            using list_iterator = typename list_type::iterator;
            using value_type = T;
            virtual ~IndexRefBase() = default;
            virtual void clear() noexcept = 0;
            virtual void erase(list_iterator i) = 0;
            virtual void insert(list_iterator i) = 0;
        protected:
            IndexRefBase() = default;
        };

        template <typename T> class IndexRef;

        template <typename K, typename T, IndexMode M>
        class IndexRef<Index<K, T, M>>:
        public IndexRefBase<T> {
        public:
            RS_NO_COPY_MOVE(IndexRef);
            using index_type = Index<K, T, M>;
            using list_type = std::list<T>;
            using list_iterator = typename list_type::iterator;
            using map_traits = typename index_type::map_traits;
            explicit IndexRef(index_type& index) noexcept: ind(index) {}
            virtual ~IndexRef() noexcept { clear(); }
            virtual void clear() noexcept { ind.map.clear(); }
            virtual void erase(list_iterator i) { ind.map.erase(ind.ext(*i)); }
            virtual void insert(list_iterator i) { map_traits::insert(ind.ext(*i), i, ind.map); }
        private:
            index_type& ind;
        };

    }

    template <typename T>
    class IndexTable {
    private:
        using list_type = std::list<T>;
    public:
        RS_NO_COPY_MOVE(IndexTable);
        using const_iterator = typename list_type::const_iterator;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
        using iterator = typename list_type::iterator;
        using reference = T&;
        using size_type = size_t;
        using value_type = T;
        IndexTable() = default;
        template <typename Range> explicit IndexTable(const Range& src);
        template <typename Iterator> IndexTable(Iterator i1, Iterator i2);
        ~IndexTable() = default;
        iterator begin() { return list.begin(); }
        const_iterator begin() const { return list.begin(); }
        void clear();
        bool empty() const noexcept { return list.empty(); }
        iterator end() { return list.end(); }
        const_iterator end() const { return list.end(); }
        void erase(iterator i);
        void erase(iterator i1, iterator i2);
        void insert(const T& t);
        template <typename Range> void insert(const Range& src) { for (auto& t: src) insert(t); }
        template <typename Iterator> void insert(Iterator i1, Iterator i2) { for (; i1 != i2; ++i1) insert(*i1); }
        void push_back(const T& t) { insert(t); }
        size_t size() const noexcept { return list.size(); }
    private:
        template <typename K, typename T2, IndexMode M> friend class Index;
        using ref_base = RS_Detail::IndexRefBase<T>;
        using ref_ptr = std::shared_ptr<ref_base>;
        using ref_map = std::unordered_map<void*, ref_ptr>;
        using ref_pair = typename ref_map::value_type;
        ref_map indices;
        list_type list;
    };

    template <typename T>
    template <typename Range>
    IndexTable<T>::IndexTable(const Range& src):
    indices(), list(begin(src), end(src)) {}

    template <typename T>
    template <typename Iterator>
    IndexTable<T>::IndexTable(Iterator i1, Iterator i2):
    indices(), list(i1, i2) {}

    template <typename T>
    void IndexTable<T>::clear() {
        list.clear();
        for (auto& pair: indices)
            pair.second->clear();
    }

    template <typename T>
    void IndexTable<T>::erase(iterator i) {
        for (auto& pair: indices)
            pair.second->erase(i);
        list.erase(i);
    }

    template <typename T>
    void IndexTable<T>::erase(iterator i1, iterator i2) {
        for (iterator i = i1; i != i2; ++i)
            for (auto& pair: indices)
                pair.second->erase(i);
        list.erase(i1, i2);
    }

    template <typename T>
    void IndexTable<T>::insert(const T& t) {
        ScopedTransaction txn;
        txn.call([&] { list.push_back(t); }, [&] { list.pop_back(); });
        iterator i = std::prev(end());
        for (auto& pair: indices)
            txn.call([&] { pair.second->insert(i); }, [&] { pair.second->erase(i); });
        txn.commit();
    }

    template <typename K, typename T, IndexMode M>
    class Index {
    private:
        RS_NO_COPY_MOVE(Index);
        using list_type = std::list<T>;
        using list_iterator = typename list_type::iterator;
        using map_traits = RS_Detail::IndexMapTraits<K, list_iterator, M>;
        using map_type = typename map_traits::map_type;
        using map_const_iterator = typename map_type::const_iterator;
        using map_iterator = typename map_type::iterator;
        template <typename CT, typename MapIter> class basic_iterator:
        public BidirectionalIterator<basic_iterator<CT, MapIter>, CT> {
        public:
            basic_iterator() = default;
            basic_iterator(const basic_iterator<T, map_iterator>& i): iter(i.iter) {}
            CT& operator*() const noexcept { return *iter->second; }
            basic_iterator& operator++() { ++iter; return *this; }
            basic_iterator& operator--() { --iter; return *this; }
            bool operator==(const basic_iterator& rhs) const noexcept { return iter == rhs.iter; }
            const K& key() const noexcept { return iter->first; }
        private:
            friend class Index;
            MapIter iter;
            basic_iterator(MapIter i): iter(i) {}
        };
    public:
        using compare_function = std::function<bool(const K&, const K&)>;
        using const_iterator = basic_iterator<const T, map_const_iterator>;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
        using extract_function = std::function<K(const T&)>;
        using iterator = basic_iterator<T, map_iterator>;
        using key_type = K;
        using reference = T&;
        using size_type = size_t;
        using table_type = IndexTable<T>;
        using value_type = T;
        static constexpr IndexMode mode = M;
        Index() = default;
        explicit Index(table_type& table);
        Index(table_type& table, extract_function extract): tab(&table), ext(extract), map(std::less<K>()) { init(); }
        Index(table_type& table, extract_function extract, compare_function compare): tab(&table), ext(extract), map(compare) { init(); }
        ~Index() noexcept { if (tab) tab->indices.erase(this); }
        iterator begin() { return map.begin(); }
        const_iterator begin() const { return map.begin(); }
        compare_function compare() const { return map.key_comp(); }
        size_t count(const K& k) const noexcept { return map.count(k); }
        bool empty() const noexcept { return map.empty(); }
        iterator end() { return map.end(); }
        const_iterator end() const { return map.end(); }
        Irange<iterator> equal_range(const K& k);
        Irange<const_iterator> equal_range(const K& k) const;
        void erase(iterator i);
        void erase(iterator i1, iterator i2);
        void erase(const K& k);
        extract_function extract() const { return ext; }
        iterator find(const K& k) { return map.find(k); }
        const_iterator find(const K& k) const { return map.find(k); }
        iterator lower_bound(const K& k) { return map.lower_bound(k); }
        const_iterator lower_bound(const K& k) const { return map.lower_bound(k); }
        size_t size() const noexcept { return map.size(); }
        table_type& table() noexcept { return *tab; }
        const table_type& table() const noexcept { return *tab; }
        iterator upper_bound(const K& k) { return map.upper_bound(k); }
        const_iterator upper_bound(const K& k) const { return map.upper_bound(k); }
    private:
        friend class IndexTable<T>;
        friend class RS_Detail::IndexRef<Index>;
        using table_iterator = typename table_type::iterator;
        using ref_type = RS_Detail::IndexRef<Index>;
        table_type* tab = nullptr;
        extract_function ext;
        map_type map;
        void init();
    };

    template <typename K, typename T, IndexMode M>
    Index<K, T, M>::Index(table_type& table):
    tab(&table), ext(), map(std::less<K>()) {
        ext = [] (const T& t) { return static_cast<K>(t); };
        init();
    }

    template <typename K, typename T, IndexMode M>
    Irange<typename Index<K, T, M>::iterator> Index<K, T, M>::equal_range(const K& k) {
        auto pair = map.equal_range(k);
        return {pair.first, pair.second};
    }

    template <typename K, typename T, IndexMode M>
    Irange<typename Index<K, T, M>::const_iterator> Index<K, T, M>::equal_range(const K& k) const {
        auto pair = map.equal_range(k);
        return {pair.first, pair.second};
    }

    template <typename K, typename T, IndexMode M>
    void Index<K, T, M>::erase(iterator i) {
        auto m = i.iter;
        auto t = m->second;
        for(auto& pair: tab->indices)
            if (pair.first != this)
                pair.second->erase(t);
        tab->list.erase(t);
        map.erase(m);
    }

    template <typename K, typename T, IndexMode M>
    void Index<K, T, M>::erase(iterator i1, iterator i2) {
        auto next = i1;
        while (i1 != i2) {
            ++next;
            erase(i1);
            i1 = next;
        }
    }

    template <typename K, typename T, IndexMode M>
    void Index<K, T, M>::erase(const K& k) {
        auto m = map.find(k);
        if (m != map.end()) {
            auto t = m->second;
            for(auto& pair: tab->indices)
                if (pair.first != this)
                    pair.second->erase(t);
            tab->list.erase(t);
            map.erase(m);
        }
    }

    template <typename K, typename T, IndexMode M>
    void Index<K, T, M>::init() {
        for (auto t = tab->begin(), e = tab->end(); t != e; ++t)
            map_traits::insert(ext(*t), t, map);
        tab->indices[this] = std::make_shared<ref_type>(*this);
    }

    // InterpolatedMap

    template <typename X, typename Y = X>
    class InterpolatedMap {
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
        InterpolatedMap() = default;
        InterpolatedMap(std::initializer_list<init_type> list);
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
    InterpolatedMap<X, Y>::InterpolatedMap(std::initializer_list<init_type> list) {
        for (auto& in: list)
            map.insert({in.x0, {in.y1, in.y2, in.y3}});
    }

    template <typename X, typename Y>
    Y InterpolatedMap<X, Y>::operator()(X x) const {
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
    void InterpolatedMap<X, Y>::erase(X x1, X x2) noexcept {
        if (x1 > x2)
            std::swap(x1, x2);
        auto i = map.lower_bound(x1);
        auto j = map.upper_bound(x2);
        map.erase(i, j);
    }

    template <typename X, typename Y>
    template <typename T>
    void InterpolatedMap<X, Y>::scale_x(T a, X b) {
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
    void InterpolatedMap<X, Y>::scale_y(T a, Y b) {
        static_assert(std::is_arithmetic<T>::value);
        for (auto& xy: map) {
            xy.second.left = a * xy.second.left + b;
            xy.second.mid = a * xy.second.mid + b;
            xy.second.right = a * xy.second.right + b;
        }
    }

    // Stack

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

RS_DEFINE_STD_HASH(RS::Blob);
