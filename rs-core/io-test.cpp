#include "rs-core/io.hpp"
#include "rs-core/string.hpp"
#include "rs-core/unit-test.hpp"
#include <set>
#include <string>

using namespace RS;
using namespace std::literals;

namespace {

    class TempFile {
    public:
        explicit TempFile(const U8string& file): f(file) {}
        ~TempFile() { remove(f.data()); }
    private:
        U8string f;
        TempFile(const TempFile&) = delete;
        TempFile(TempFile&&) = delete;
        TempFile& operator=(const TempFile&) = delete;
        TempFile& operator=(TempFile&&) = delete;
    };

    void check_file_io_operations() {

        std::string s, readme = "README.md", testfile = "__test__", nofile = "__no_such_file__";
        TempFile tempfile(testfile);

        TEST(load_file(readme, s));
        TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
        TEST(load_file(readme, s, 100));
        TEST_EQUAL(s.size(), 100);
        TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
        TEST(load_file(readme, s, 10));
        TEST_EQUAL(s.size(), 10);
        TEST_EQUAL(s.substr(0, 20), "# RS Core ");

        TEST(save_file(testfile, "Hello world\n"s));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\n");
        TEST(save_file(testfile, "Goodbye\n"s, true));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\nGoodbye\n");
        TEST(! load_file(nofile, s));

        #ifndef _XOPEN_SOURCE

            std::wstring wreadme = L"README.md", wtestfile = L"__test__", wnofile = L"__no_such_file__";

            TEST(load_file(wreadme, s));
            TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
            TEST(load_file(wreadme, s, 100));
            TEST_EQUAL(s.size(), 100);
            TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
            TEST(load_file(wreadme, s, 10));
            TEST_EQUAL(s.size(), 10);
            TEST_EQUAL(s.substr(0, 20), "# RS Core ");

            TEST(save_file(wtestfile, "Hello world\n"s));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\n");
            TEST(save_file(wtestfile, "Goodbye\n"s, true));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\nGoodbye\n");
            TEST(! load_file(wnofile, s));

        #endif

    }

    void check_process_io_operations() {

        U8string s;

        #ifdef _XOPEN_SOURCE
            TRY(s = shell("ls"));
        #else
            TRY(s = shell("dir /b"));
        #endif

        TEST(! s.empty());
        std::set<U8string> ss;
        TRY(split(s, append(ss), "\r\n"));
        TEST(! ss.empty());
        TEST(ss.count("Makefile"));
        TEST(ss.count("build"));

    }

}

TEST_MODULE(core, io) {

    check_file_io_operations();
    check_process_io_operations();

}
