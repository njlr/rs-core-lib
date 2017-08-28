#include "rs-core/container.hpp"
#include "rs-core/string.hpp"
#include "rs-core/unit-test.hpp"
#include "rs-core/vector.hpp"
#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace RS;

namespace {

    void check_array_map() {

        ArrayMap<int, U8string> map;
        ArrayMap<int, U8string>::iterator i;
        U8string s;

        TEST(map.empty());
        TEST_EQUAL(map.size(), 0);
        TEST(! map.has(10));
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{}");

        TEST(map.insert({30, "hello"}).second);
        TEST(! map.empty());
        TEST_EQUAL(map.size(), 1);
        TEST(map.has(30));
        TEST(! map.has(20));
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{30:hello}");

        TEST(map.insert({20, "world"}).second);
        TEST(map.insert({10, "goodbye"}).second);
        TEST(! map.empty());
        TEST_EQUAL(map.size(), 3);
        TEST(map.has(30));
        TEST(map.has(20));
        TEST(map.has(10));
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{30:hello,20:world,10:goodbye}");

        TRY(map[30] = "foo");
        TRY(map[40] = "bar");
        TEST_EQUAL(map.size(), 4);
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{30:foo,20:world,10:goodbye,40:bar}");

        TEST(! map.insert({30, "hello again"}).second);
        TEST_EQUAL(map.size(), 4);
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{30:foo,20:world,10:goodbye,40:bar}");

        TRY(i = map.find(10));
        REQUIRE(i != map.end());
        TEST_EQUAL(i->first, 10);
        TEST_EQUAL(i->second, "goodbye");

        TRY(i = map.find(50));
        TEST(i == map.end());

        TRY(map.erase(map.begin()));
        TEST_EQUAL(map.size(), 3);
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{20:world,10:goodbye,40:bar}");

        TEST(map.erase(10));
        TEST_EQUAL(map.size(), 2);
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{20:world,40:bar}");

        TEST(! map.erase(30));
        TEST_EQUAL(map.size(), 2);
        TRY(s = to_str(map));
        TEST_EQUAL(s, "{20:world,40:bar}");

        TRY(map.clear());
        TEST(map.empty());

    }

    void check_array_set() {

        ArraySet<U8string> set;
        ArraySet<U8string>::iterator i;
        U8string s;

        TEST(set.empty());
        TEST_EQUAL(set.size(), 0);
        TEST(! set.has("hello"));
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[]");

        TEST(set.insert("hello").second);
        TEST(! set.empty());
        TEST_EQUAL(set.size(), 1);
        TEST(set.has("hello"));
        TEST(! set.has("goodbye"));
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[hello]");

        TEST(set.insert("world").second);
        TEST(set.insert("goodbye").second);
        TEST(! set.empty());
        TEST_EQUAL(set.size(), 3);
        TEST(set.has("hello"));
        TEST(set.has("goodbye"));
        TEST(set.has("world"));
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[hello,world,goodbye]");

        TEST(! set.insert("hello").second);

        TRY(i = set.find("world"));
        REQUIRE(i != set.end());
        TEST_EQUAL(*i, "world");

        TRY(i = set.find("foobar"));
        TEST(i == set.end());

        TRY(set.erase(set.begin()));
        TEST_EQUAL(set.size(), 2);
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[world,goodbye]");

        TEST(set.erase("goodbye"));
        TEST_EQUAL(set.size(), 1);
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[world]");

        TEST(! set.erase("hello"));
        TEST_EQUAL(set.size(), 1);
        TRY(s = to_str(set));
        TEST_EQUAL(s, "[world]");

        TRY(set.clear());
        TEST(set.empty());

    }

    void check_blob() {

        Blob b;

        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);
        TEST_EQUAL(b.hex(), "");
        TEST_EQUAL(b.str(), "");
        TEST_EQUAL(U8string(b.chars().begin(), b.chars().end()), "");

        TRY(b = Blob(nullptr, 1));
        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);

        TRY(b = Blob(5, 'a'));
        TEST(! b.empty());
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "61 61 61 61 61");
        TEST_EQUAL(b.str(), "aaaaa");
        TEST_EQUAL(U8string(b.chars().begin(), b.chars().end()), "aaaaa");

        TRY(b.fill('z'));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "7a 7a 7a 7a 7a");
        TEST_EQUAL(b.str(), "zzzzz");

        TRY(b.clear());
        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);
        TEST_EQUAL(b.hex(), "");
        TEST_EQUAL(b.str(), "");

        TRY(b.reset(10, 'a'));
        TEST(! b.empty());
        TEST_EQUAL(b.size(), 10);
        TEST_EQUAL(b.hex(), "61 61 61 61 61 61 61 61 61 61");
        TEST_EQUAL(b.str(), "aaaaaaaaaa");

        TRY(b.copy("hello", 5));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "68 65 6c 6c 6f");
        TEST_EQUAL(b.str(), "hello");

        U8string s, h = "hello", w = "world";

        TRY(b.clear());
        TRY(b.reset(&h[0], 5, [&] (void*) { s += "a"; }));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.str(), "hello");
        TEST_EQUAL(s, "");
        TRY(b = Blob(&w[0], 5, [&] (void*) { s += "b"; }));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.str(), "world");
        TEST_EQUAL(s, "a");
        TRY(b.clear());
        TEST_EQUAL(s, "ab");

    }

    template <typename T, size_t N>
    U8string grid_format_helper(const Grid<T, N>& g, Vector<ptrdiff_t, N> index, size_t current) {
        std::ostringstream out;
        out << "[";
        ptrdiff_t n = g.shape()[current];
        ptrdiff_t& i(index[current]);
        for (i = 0; i < n; ++i) {
            if (current == N - 1)
                out << g[index];
            else
                out << grid_format_helper(g, index, current + 1);
            out << ",";
        }
        auto str = out.str();
        if (n > 0)
            str.pop_back();
        str += "]";
        return str;
    }

    template <typename T, size_t N>
    U8string grid_format(const Grid<T, N>& g) {
        Vector<ptrdiff_t, N> index;
        std::fill(index.begin(), index.end(), 0);
        return grid_format_helper(g, index, 0);
    }

    void check_grid() {

        using grid_i2 = Grid<int, 2>;
        using grid_i4 = Grid<int, 4>;
        using grid_s2 = Grid<std::string, 2>;

        grid_i2 ig;
        grid_i4 ig4;
        grid_i2::iterator i;
        grid_i2::const_iterator ci;
        grid_s2 sg;

        TEST(sg.empty());
        TEST_EQUAL(sg.size(), 0);
        TEST_EQUAL(to_str(sg.shape()), "[0,0]");
        TEST_EQUAL(grid_format(sg), "[]");

        TRY(sg = grid_s2(3, 4));
        TEST(! sg.empty());
        TEST_EQUAL(sg.size(), 12);
        TEST_EQUAL(to_str(sg.shape()), "[3,4]");

        TRY(sg.fill("@"));
        TEST_EQUAL(grid_format(sg), "[[@,@,@,@],[@,@,@,@],[@,@,@,@]]");

        TRY(sg = grid_s2({4, 3}, "#"));
        TEST_EQUAL(to_str(sg.shape()), "[4,3]");
        TEST_EQUAL(grid_format(sg), "[[#,#,#],[#,#,#],[#,#,#],[#,#,#]]");

        TRY(sg.reset(3, 4));
        TEST_EQUAL(to_str(sg.shape()), "[3,4]");

        TRY(sg.fill("@"));
        TEST_EQUAL(grid_format(sg), "[[@,@,@,@],[@,@,@,@],[@,@,@,@]]");

        TRY(sg.reset({4, 3}, "#"));
        TEST_EQUAL(to_str(sg.shape()), "[4,3]");
        TEST_EQUAL(grid_format(sg), "[[#,#,#],[#,#,#],[#,#,#],[#,#,#]]");

        TRY(sg.clear());
        TEST(sg.empty());
        TEST_EQUAL(sg.size(), 0);
        TEST_EQUAL(to_str(sg.shape()), "[0,0]");

        TRY(ig.reset(4, 3));
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 3; ++j)
                TRY(ig.at(i, j) = 100 + 10 * i + j);
        TEST_EQUAL(grid_format(ig), "[[100,101,102],[110,111,112],[120,121,122],[130,131,132]]");
        TEST_EQUAL(to_str(ig), "[100,101,102,110,111,112,120,121,122,130,131,132]");
        TEST_EQUAL(ig.at(0, 0), 100);
        TEST_EQUAL(ig.at(0, 2), 102);
        TEST_EQUAL(ig.at(3, 0), 130);
        TEST_EQUAL(ig.at(3, 2), 132);
        TEST_THROW(ig.at(0, 3), std::out_of_range);
        TEST_THROW(ig.at(4, 0), std::out_of_range);
        TEST(ig.contains(0, 0));
        TEST(ig.contains(3, 2));
        TEST(! ig.contains(3, 3));
        TEST(! ig.contains(4, 2));

        TRY(i = ig.locate(1, 1));
        TEST_EQUAL(*i, 111);
        TEST_EQUAL(to_str(i.pos()), "[1,1]");

        TRY(ci = ig.locate(1, 1));
        TEST_EQUAL(*ci, 111);
        TEST_EQUAL(to_str(ci.pos()), "[1,1]");

        TRY(i.move(0, 1));
        TEST_EQUAL(*i, 121);
        TEST_EQUAL(to_str(i.pos()), "[2,1]");

        TRY(i.move(1, -1));
        TEST_EQUAL(*i, 120);
        TEST_EQUAL(to_str(i.pos()), "[2,0]");

        TRY(ig.reshape(2, 5));
        TEST_EQUAL(ig.size(), 10);
        TEST_EQUAL(to_str(ig.shape()), "[2,5]");
        TEST_EQUAL(grid_format(ig), "[[100,101,102,0,0],[110,111,112,0,0]]");

        TRY(ig4.reset(2, 3, 2, 3));
        TEST_EQUAL(ig4.size(), 36);
        TEST_EQUAL(to_str(ig4.shape()), "[2,3,2,3]");
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 2; ++k)
                    for (int l = 0; l < 3; ++l)
                        TRY(ig4(i, j, k, l) = 1111 + 1000 * i + 100 * j + 10 * k + l);
        TEST_EQUAL(grid_format(ig4),
            "[[[[1111,1112,1113],[1121,1122,1123]],"
              "[[1211,1212,1213],[1221,1222,1223]],"
              "[[1311,1312,1313],[1321,1322,1323]]],"
             "[[[2111,2112,2113],[2121,2122,2123]],"
              "[[2211,2212,2213],[2221,2222,2223]],"
              "[[2311,2312,2313],[2321,2322,2323]]]]"
        );

        TRY(ig4.reshape(4, 2, 4, 2));
        TEST_EQUAL(ig4.size(), 64);
        TEST_EQUAL(to_str(ig4.shape()), "[4,2,4,2]");
        TEST_EQUAL(grid_format(ig4),
            "[[[[1111,1112],[1121,1122],[0,0],[0,0]],"
              "[[1211,1212],[1221,1222],[0,0],[0,0]]],"
             "[[[2111,2112],[2121,2122],[0,0],[0,0]],"
              "[[2211,2212],[2221,2222],[0,0],[0,0]]],"
             "[[[0,0],[0,0],[0,0],[0,0]],"
              "[[0,0],[0,0],[0,0],[0,0]]],"
             "[[[0,0],[0,0],[0,0],[0,0]],"
              "[[0,0],[0,0],[0,0],[0,0]]]]"
        );

    }

    class Neddie:
    public EqualityComparable<Neddie> {
    public:
        Neddie(): n(0), s() {}
        Neddie(int num, const std::string& str): n(num), s(str) {}
        int num() const { return n; }
        std::string str() const { return s; }
        bool operator==(const Neddie& rhs) const { return n == rhs.n && s == rhs.s; }
    private:
        int n;
        std::string s;
    };

    std::ostream& operator<<(std::ostream& out, const Neddie& x) { return out << x.num() << ":" << x.str(); }

    void check_index() {

        using table_type = IndexTable<Neddie>;
        using int_index = Index<int, Neddie, IndexMode::duplicate>;
        using string_index = Index<std::string, Neddie>;

        table_type t1, t2;
        const table_type& ct1(t1);
        std::unique_ptr<int_index> iip;
        std::unique_ptr<string_index> sip;
        table_type::iterator x1, x2;
        int_index::iterator y1, y2;
        int_index::const_iterator cy1, cy2;
        string_index::iterator z1, z2;
        string_index::const_iterator cz1, cz2;

        TEST(t1.empty());
        TEST_EQUAL(t1.size(), 0);
        TRY(iip = std::make_unique<int_index>(t1, &Neddie::num));
        TRY(sip = std::make_unique<string_index>(t1, &Neddie::str, [] (std::string a, std::string b) { return a > b; }));

        int_index& ii = *iip;
        string_index& si = *sip;
        const int_index& cii = *iip;
        const string_index& csi = *sip;

        TEST(! cii.compare()(2, 1));
        TEST(! cii.compare()(2, 2));
        TEST(cii.compare()(2, 3));
        TEST_EQUAL(cii.extract()(Neddie(1, "zulu")), 1);
        TEST(! csi.compare()("yankee", "zulu"));
        TEST(! csi.compare()("yankee", "yankee"));
        TEST(csi.compare()("yankee", "xray"));
        TEST_EQUAL(csi.extract()(Neddie(1, "zulu")), "zulu");
        TEST(ii.empty());
        TEST(si.empty());
        TEST_EQUAL(ii.size(), 0);
        TEST_EQUAL(si.size(), 0);

        void* a0 = &t1;
        void* a1 = nullptr;
        const void* a2 = nullptr;
        void* a3 = nullptr;
        const void* a4 = nullptr;

        TRY(a1 = &ii.table());
        TRY(a2 = &cii.table());
        TRY(a3 = &si.table());
        TRY(a4 = &csi.table());
        TEST_EQUAL(a1, a0);
        TEST_EQUAL(a2, a0);
        TEST_EQUAL(a3, a0);
        TEST_EQUAL(a4, a0);

        TRY(t1.insert(Neddie(1, "alpha")));
        TRY(t1.insert(Neddie(2, "bravo")));
        TRY(t1.insert(Neddie(3, "charlie")));
        TRY(t1.insert(Neddie(4, "delta")));
        TRY(t1.insert(Neddie(5, "echo")));
        TEST(! t1.empty());
        TEST(! cii.empty());
        TEST(! csi.empty());
        TEST_EQUAL(t1.size(), 5);
        TEST_EQUAL(cii.size(), 5);
        TEST_EQUAL(csi.size(), 5);
        TEST_EQUAL(cii.count(1), 1);
        TEST_EQUAL(cii.count(10), 0);
        TEST_EQUAL(csi.count("alpha"), 1);
        TEST_EQUAL(csi.count("zulu"), 0);
        TEST_EQUAL(*cii.begin(), Neddie(1, "alpha"));
        TEST_EQUAL(*csi.begin(), Neddie(5, "echo"));
        TEST_EQUAL(cii.begin().key(), 1);
        TEST_EQUAL(csi.begin().key(), "echo");
        TEST_EQUAL(ii.begin().key(), 1);
        TEST_EQUAL(si.begin().key(), "echo");

        TRY(z1 = si.find("echo"));
        TEST(z1 == si.begin());
        TEST(z1 != si.end());
        TEST_EQUAL(*z1, Neddie(5, "echo"));
        TEST_EQUAL(z1.key(), "echo");
        TRY(z1 = si.find("zulu"));
        TEST(z1 == si.end());
        TRY(cz1 = csi.find("echo"));
        TEST(cz1 == csi.begin());
        TEST(cz1 != csi.end());
        TEST_EQUAL(*cz1, Neddie(5, "echo"));
        TEST_EQUAL(cz1.key(), "echo");
        TRY(cz1 = csi.find("zulu"));
        TEST(cz1 == csi.end());

        TEST_EQUAL(to_str(t1), "[1:alpha,2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(ct1), "[1:alpha,2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(ii), "[1:alpha,2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(cii), "[1:alpha,2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(si), "[5:echo,4:delta,3:charlie,2:bravo,1:alpha]");
        TEST_EQUAL(to_str(csi), "[5:echo,4:delta,3:charlie,2:bravo,1:alpha]");

        TRY(t1.erase(t1.begin()));
        TEST_EQUAL(t1.size(), 4);
        TEST_EQUAL(cii.size(), 4);
        TEST_EQUAL(csi.size(), 4);
        TEST_EQUAL(to_str(ct1), "[2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(cii), "[2:bravo,3:charlie,4:delta,5:echo]");
        TEST_EQUAL(to_str(csi), "[5:echo,4:delta,3:charlie,2:bravo]");

        TRY(si.erase(si.begin()));
        TEST_EQUAL(t1.size(), 3);
        TEST_EQUAL(cii.size(), 3);
        TEST_EQUAL(csi.size(), 3);
        TEST_EQUAL(to_str(ct1), "[2:bravo,3:charlie,4:delta]");
        TEST_EQUAL(to_str(cii), "[2:bravo,3:charlie,4:delta]");
        TEST_EQUAL(to_str(csi), "[4:delta,3:charlie,2:bravo]");

        TRY(ii.erase(3));
        TEST_EQUAL(t1.size(), 2);
        TEST_EQUAL(cii.size(), 2);
        TEST_EQUAL(csi.size(), 2);
        TEST_EQUAL(to_str(ct1), "[2:bravo,4:delta]");
        TEST_EQUAL(to_str(cii), "[2:bravo,4:delta]");
        TEST_EQUAL(to_str(csi), "[4:delta,2:bravo]");

        TRY(t1.insert(Neddie(2, "zulu")));
        TEST_THROW(t1.insert(Neddie(1, "bravo")), IndexCollision);

        TRY(t1.clear());
        TEST(t1.empty());
        TEST(cii.empty());
        TEST(csi.empty());
        TEST_EQUAL(t1.size(), 0);
        TEST_EQUAL(cii.size(), 0);
        TEST_EQUAL(csi.size(), 0);

        TRY(t1.push_back(Neddie(1, "alpha")));
        TRY(t1.push_back(Neddie(2, "bravo")));
        TRY(t1.push_back(Neddie(3, "charlie")));
        TRY(t1.push_back(Neddie(4, "delta")));
        TRY(t1.push_back(Neddie(5, "echo")));
        TRY(t1.push_back(Neddie(6, "foxtrot")));
        TRY(t1.push_back(Neddie(7, "golf")));
        TRY(t1.push_back(Neddie(8, "hotel")));
        TRY(t1.push_back(Neddie(9, "india")));
        TRY(t1.push_back(Neddie(10, "juliet")));
        TEST_EQUAL(t1.size(), 10);
        TEST_EQUAL(cii.size(), 10);
        TEST_EQUAL(csi.size(), 10);

        TRY(x1 = t1.begin());
        TRY(x2 = t1.begin());
        TRY(std::advance(x1, 1));
        TRY(std::advance(x2, 4));
        TRY(t1.erase(x1, x2));
        TEST_EQUAL(t1.size(), 7);
        TEST_EQUAL(cii.size(), 7);
        TEST_EQUAL(csi.size(), 7);
        TEST_EQUAL(to_str(ct1), "[1:alpha,5:echo,6:foxtrot,7:golf,8:hotel,9:india,10:juliet]");

        TRY(y1 = ii.begin());
        TRY(y2 = ii.begin());
        TRY(std::advance(y1, 1));
        TRY(std::advance(y2, 3));
        TRY(ii.erase(y1, y2));
        TEST_EQUAL(t1.size(), 5);
        TEST_EQUAL(cii.size(), 5);
        TEST_EQUAL(csi.size(), 5);
        TEST_EQUAL(to_str(ct1), "[1:alpha,7:golf,8:hotel,9:india,10:juliet]");

        TRY(z1 = si.begin());
        TRY(z2 = si.begin());
        TRY(std::advance(z1, 1));
        TRY(std::advance(z2, 3));
        TRY(si.erase(z1, z2));
        TEST_EQUAL(t1.size(), 3);
        TEST_EQUAL(cii.size(), 3);
        TEST_EQUAL(csi.size(), 3);
        TEST_EQUAL(to_str(ct1), "[1:alpha,7:golf,10:juliet]");

        TRY(t1.clear());
        TRY(t1.insert(Neddie(1, "zulu")));
        TRY(t1.insert(Neddie(1, "yankee")));
        TRY(t1.insert(Neddie(2, "xray")));
        TRY(t1.insert(Neddie(2, "whiskey")));
        TRY(t1.insert(Neddie(99, "victor")));
        TRY(t1.insert(Neddie(99, "uniform")));
        TEST_EQUAL(t1.size(), 6);
        TEST_EQUAL(cii.size(), 6);
        TEST_EQUAL(csi.size(), 6);

        TRY(y1 = ii.lower_bound(2));               TEST_EQUAL(std::distance(ii.begin(), y1), 2);
        TRY(y1 = ii.upper_bound(2));               TEST_EQUAL(std::distance(ii.begin(), y1), 4);
        TRY(y1 = ii.equal_range(2).first);         TEST_EQUAL(std::distance(ii.begin(), y1), 2);
        TRY(y1 = ii.equal_range(2).second);        TEST_EQUAL(std::distance(ii.begin(), y1), 4);
        TRY(cy1 = cii.lower_bound(20));            TEST_EQUAL(std::distance(cii.begin(), cy1), 4);
        TRY(cy1 = cii.upper_bound(20));            TEST_EQUAL(std::distance(cii.begin(), cy1), 4);
        TRY(cy1 = cii.equal_range(20).first);      TEST_EQUAL(std::distance(cii.begin(), cy1), 4);
        TRY(cy1 = cii.equal_range(20).second);     TEST_EQUAL(std::distance(cii.begin(), cy1), 4);
        TRY(z1 = si.lower_bound("xray"));          TEST_EQUAL(std::distance(si.begin(), z1), 2);
        TRY(z1 = si.upper_bound("xray"));          TEST_EQUAL(std::distance(si.begin(), z1), 3);
        TRY(z1 = si.equal_range("xray").first);    TEST_EQUAL(std::distance(si.begin(), z1), 2);
        TRY(z1 = si.equal_range("xray").second);   TEST_EQUAL(std::distance(si.begin(), z1), 3);
        TRY(cz1 = csi.lower_bound("xyz"));         TEST_EQUAL(std::distance(csi.begin(), cz1), 2);
        TRY(cz1 = csi.upper_bound("xyz"));         TEST_EQUAL(std::distance(csi.begin(), cz1), 2);
        TRY(cz1 = csi.equal_range("xyz").first);   TEST_EQUAL(std::distance(csi.begin(), cz1), 2);
        TRY(cz1 = csi.equal_range("xyz").second);  TEST_EQUAL(std::distance(csi.begin(), cz1), 2);

        TRY(t2.insert(Neddie(1, "zulu")));
        TRY(t2.insert(Neddie(1, "zulu")));
        TEST_EQUAL(t2.size(), 2);
        TRY(int_index(t2, &Neddie::num));
        TEST_THROW(string_index(t2, &Neddie::str), IndexCollision);

    }

    void check_scale_map() {

        ScaleMap<double> sm;

        TEST(sm.empty());
        TEST_EQUAL(sm.min(), 0);
        TEST_EQUAL(sm.max(), 0);
        TEST_EQUAL(sm(0), 0);
        TEST_EQUAL(sm(10), 0);

        TRY(sm.insert(5, 10));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 5);
        TEST_EQUAL(sm.max(), 5);
        TEST_EQUAL(sm(0), 10);
        TEST_EQUAL(sm(5), 10);
        TEST_EQUAL(sm(10), 10);

        TRY(sm.insert(10, 20));
        TEST_EQUAL(sm.min(), 5);
        TEST_EQUAL(sm.max(), 10);
        TEST_EQUAL(sm(4), 10);
        TEST_EQUAL(sm(5), 10);
        TEST_EQUAL(sm(6), 12);
        TEST_EQUAL(sm(7), 14);
        TEST_EQUAL(sm(8), 16);
        TEST_EQUAL(sm(9), 18);
        TEST_EQUAL(sm(10), 20);
        TEST_EQUAL(sm(11), 20);

        TRY(sm.insert(15, 100, 10));
        TRY(sm.insert(20, -10));
        TEST_EQUAL(sm.min(), 5);
        TEST_EQUAL(sm.max(), 20);
        TEST_EQUAL(sm(4), 10);
        TEST_EQUAL(sm(5), 10);
        TEST_EQUAL(sm(6), 12);
        TEST_EQUAL(sm(7), 14);
        TEST_EQUAL(sm(8), 16);
        TEST_EQUAL(sm(9), 18);
        TEST_EQUAL(sm(10), 20);
        TEST_EQUAL(sm(11), 36);
        TEST_EQUAL(sm(12), 52);
        TEST_EQUAL(sm(13), 68);
        TEST_EQUAL(sm(14), 84);
        TEST_EQUAL(sm(15), 55);
        TEST_EQUAL(sm(16), 6);
        TEST_EQUAL(sm(17), 2);
        TEST_EQUAL(sm(18), -2);
        TEST_EQUAL(sm(19), -6);
        TEST_EQUAL(sm(20), -10);
        TEST_EQUAL(sm(21), -10);

        TRY(sm.erase(10));
        TEST_EQUAL(sm.min(), 5);
        TEST_EQUAL(sm.max(), 20);
        TEST_EQUAL(sm(4), 10);
        TEST_EQUAL(sm(5), 10);
        TEST_EQUAL(sm(6), 19);
        TEST_EQUAL(sm(7), 28);
        TEST_EQUAL(sm(8), 37);
        TEST_EQUAL(sm(9), 46);
        TEST_EQUAL(sm(10), 55);
        TEST_EQUAL(sm(11), 64);
        TEST_EQUAL(sm(12), 73);
        TEST_EQUAL(sm(13), 82);
        TEST_EQUAL(sm(14), 91);
        TEST_EQUAL(sm(15), 55);
        TEST_EQUAL(sm(16), 6);
        TEST_EQUAL(sm(17), 2);
        TEST_EQUAL(sm(18), -2);
        TEST_EQUAL(sm(19), -6);
        TEST_EQUAL(sm(20), -10);
        TEST_EQUAL(sm(21), -10);

        TRY(sm.erase(5, 15));
        TEST_EQUAL(sm.min(), 20);
        TEST_EQUAL(sm.max(), 20);
        TEST_EQUAL(sm(4), -10);
        TEST_EQUAL(sm(5), -10);
        TEST_EQUAL(sm(6), -10);
        TEST_EQUAL(sm(7), -10);
        TEST_EQUAL(sm(8), -10);
        TEST_EQUAL(sm(9), -10);
        TEST_EQUAL(sm(10), -10);
        TEST_EQUAL(sm(11), -10);
        TEST_EQUAL(sm(12), -10);
        TEST_EQUAL(sm(13), -10);
        TEST_EQUAL(sm(14), -10);
        TEST_EQUAL(sm(15), -10);
        TEST_EQUAL(sm(16), -10);
        TEST_EQUAL(sm(17), -10);
        TEST_EQUAL(sm(18), -10);
        TEST_EQUAL(sm(19), -10);
        TEST_EQUAL(sm(20), -10);
        TEST_EQUAL(sm(21), -10);

        TRY(sm.erase(0, 100));
        TEST(sm.empty());
        TEST_EQUAL(sm.min(), 0);
        TEST_EQUAL(sm.max(), 0);
        TEST_EQUAL(sm(0), 0);
        TEST_EQUAL(sm(10), 0);

        TRY((sm = {
            {10, 100},
            {20, 200, 300, 400},
            {30, 500, 600},
        }));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 10);
        TEST_EQUAL(sm.max(), 30);
        TEST_EQUAL(sm(5), 100);
        TEST_EQUAL(sm(10), 100);
        TEST_EQUAL(sm(15), 150);
        TEST_EQUAL(sm(20), 300);
        TEST_EQUAL(sm(25), 450);
        TEST_EQUAL(sm(30), 550);
        TEST_EQUAL(sm(35), 600);

        TRY(sm.scale_x(2, 100));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 120);
        TEST_EQUAL(sm.max(), 160);
        TEST_EQUAL(sm(110), 100);
        TEST_EQUAL(sm(120), 100);
        TEST_EQUAL(sm(130), 150);
        TEST_EQUAL(sm(140), 300);
        TEST_EQUAL(sm(150), 450);
        TEST_EQUAL(sm(160), 550);
        TEST_EQUAL(sm(170), 600);

        TRY(sm.scale_x(-0.5, 100));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 20);
        TEST_EQUAL(sm.max(), 40);
        TEST_EQUAL(sm(15), 600);
        TEST_EQUAL(sm(20), 550);
        TEST_EQUAL(sm(25), 450);
        TEST_EQUAL(sm(30), 300);
        TEST_EQUAL(sm(35), 150);
        TEST_EQUAL(sm(40), 100);
        TEST_EQUAL(sm(45), 100);

        TRY(sm.scale_y(0.5, 1000));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 20);
        TEST_EQUAL(sm.max(), 40);
        TEST_EQUAL(sm(15), 1300);
        TEST_EQUAL(sm(20), 1275);
        TEST_EQUAL(sm(25), 1225);
        TEST_EQUAL(sm(30), 1150);
        TEST_EQUAL(sm(35), 1075);
        TEST_EQUAL(sm(40), 1050);
        TEST_EQUAL(sm(45), 1050);

        TRY(sm.scale_x(0));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 0);
        TEST_EQUAL(sm.max(), 0);
        TEST_EQUAL(sm(-5), 1300);
        TEST_EQUAL(sm(0), 1175);
        TEST_EQUAL(sm(5), 1050);

        TRY(sm.clear());
        TEST(sm.empty());
        TEST_EQUAL(sm.min(), 0);
        TEST_EQUAL(sm.max(), 0);
        TEST_EQUAL(sm(0), 0);
        TEST_EQUAL(sm(10), 0);

        TRY(sm.insert(20, 25, 75, 100));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 20);
        TEST_EQUAL(sm.max(), 20);
        TEST_EQUAL(sm(15), 25);
        TEST_EQUAL(sm(20), 75);
        TEST_EQUAL(sm(25), 100);

        TRY(sm.scale_x(2));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), 40);
        TEST_EQUAL(sm.max(), 40);
        TEST_EQUAL(sm(35), 25);
        TEST_EQUAL(sm(40), 75);
        TEST_EQUAL(sm(45), 100);

        TRY(sm.scale_x(-2));
        TEST(! sm.empty());
        TEST_EQUAL(sm.min(), -80);
        TEST_EQUAL(sm.max(), -80);
        TEST_EQUAL(sm(-85), 100);
        TEST_EQUAL(sm(-80), 75);
        TEST_EQUAL(sm(-75), 25);

    }

    class TopTail {
    public:
        TopTail(): sp(nullptr), ch() {}
        TopTail(U8string& s, char c): sp(&s), ch(c) { s += '+'; s += c; }
        ~TopTail() { term(); }
        TopTail(TopTail&& t): sp(t.sp), ch(t.ch) { t.sp = nullptr; }
        TopTail& operator=(TopTail&& t) { if (&t != this) { term(); sp = t.sp; ch = t.ch; t.sp = nullptr; } return *this; }
        char get() const noexcept { return ch; }
        friend std::ostream& operator<<(std::ostream& out, const TopTail& t) { return out << t.ch; }
    private:
        U8string* sp;
        char ch;
        TopTail(const TopTail&) = delete;
        TopTail& operator=(const TopTail&) = delete;
        void term() { if (sp) { *sp += '-'; *sp += ch; sp = nullptr; } }
    };

    void check_stack() {

        Stack<TopTail> st;
        U8string s;

        TEST(st.empty());
        TEST_EQUAL(to_str(st), "[]");
        TRY(st.emplace(s, 'a'));
        TEST_EQUAL(st.size(), 1);
        TEST_EQUAL(st.top().get(), 'a');
        TEST_EQUAL(to_str(st), "[a]");
        TEST_EQUAL(s, "+a");
        TRY(st.emplace(s, 'b'));
        TEST_EQUAL(st.size(), 2);
        TEST_EQUAL(st.top().get(), 'b');
        TEST_EQUAL(to_str(st), "[a,b]");
        TEST_EQUAL(s, "+a+b");
        TRY(st.emplace(s, 'c'));
        TEST_EQUAL(st.size(), 3);
        TEST_EQUAL(st.top().get(), 'c');
        TEST_EQUAL(to_str(st), "[a,b,c]");
        TEST_EQUAL(s, "+a+b+c");
        TRY(st.clear());
        TEST(st.empty());
        TEST_EQUAL(to_str(st), "[]");
        TEST_EQUAL(s, "+a+b+c-c-b-a");

    }

}

TEST_MODULE(core, container) {

    check_array_map();
    check_array_set();
    check_blob();
    check_grid();
    check_index();
    check_scale_map();
    check_stack();

}
