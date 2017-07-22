#include "rs-core/container.hpp"
#include "rs-core/unit-test.hpp"

using namespace RS;

namespace {

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

    class TopTail {
    public:
        TopTail(): sp(nullptr), ch() {}
        TopTail(U8string& s, char c): sp(&s), ch(c) { s += '+'; s += c; }
        ~TopTail() { term(); }
        TopTail(TopTail&& t): sp(t.sp), ch(t.ch) { t.sp = nullptr; }
        TopTail& operator=(TopTail&& t) { if (&t != this) { term(); sp = t.sp; ch = t.ch; t.sp = nullptr; } return *this; }
        char get() const noexcept { return ch; }
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
        TRY(st.push(TopTail(s, 'a')));
        TEST_EQUAL(st.size(), 1);
        TEST_EQUAL(st.top().get(), 'a');
        TEST_EQUAL(s, "+a");
        TRY(st.push(TopTail(s, 'b')));
        TEST_EQUAL(st.size(), 2);
        TEST_EQUAL(st.top().get(), 'b');
        TEST_EQUAL(s, "+a+b");
        TRY(st.push(TopTail(s, 'c')));
        TEST_EQUAL(st.size(), 3);
        TEST_EQUAL(st.top().get(), 'c');
        TEST_EQUAL(s, "+a+b+c");
        TRY(st.clear());
        TEST(st.empty());
        TEST_EQUAL(s, "+a+b+c-c-b-a");

    }

}

TEST_MODULE(core, container) {

    check_blob();
    check_stack();

}
