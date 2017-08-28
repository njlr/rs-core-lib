#pragma once

#include "rs-core/common.hpp"
#include "rs-core/vector.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace RS {

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

}
