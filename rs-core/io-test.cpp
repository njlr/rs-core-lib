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
            TRY(s = run_command("ls"));
        #else
            TRY(s = run_command("dir /b"));
        #endif

        TEST(! s.empty());
        std::set<U8string> ss;
        TRY(split(s, append(ss), "\r\n"));
        TEST(! ss.empty());
        TEST(ss.count("Makefile"));
        TEST(ss.count("build"));

    }

    void check_terminal_io_operations() {

        U8string s;

        TRY(s = xt_up);              TEST_EQUAL(s, "\x1b[A");
        TRY(s = xt_down);            TEST_EQUAL(s, "\x1b[B");
        TRY(s = xt_right);           TEST_EQUAL(s, "\x1b[C");
        TRY(s = xt_left);            TEST_EQUAL(s, "\x1b[D");
        TRY(s = xt_erase_left);      TEST_EQUAL(s, "\x1b[1K");
        TRY(s = xt_erase_right);     TEST_EQUAL(s, "\x1b[K");
        TRY(s = xt_erase_above);     TEST_EQUAL(s, "\x1b[1J");
        TRY(s = xt_erase_below);     TEST_EQUAL(s, "\x1b[J");
        TRY(s = xt_erase_line);      TEST_EQUAL(s, "\x1b[2K");
        TRY(s = xt_clear);           TEST_EQUAL(s, "\x1b[2J");
        TRY(s = xt_reset);           TEST_EQUAL(s, "\x1b[0m");
        TRY(s = xt_bold);            TEST_EQUAL(s, "\x1b[1m");
        TRY(s = xt_under);           TEST_EQUAL(s, "\x1b[4m");
        TRY(s = xt_black);           TEST_EQUAL(s, "\x1b[30m");
        TRY(s = xt_red);             TEST_EQUAL(s, "\x1b[31m");
        TRY(s = xt_green);           TEST_EQUAL(s, "\x1b[32m");
        TRY(s = xt_yellow);          TEST_EQUAL(s, "\x1b[33m");
        TRY(s = xt_blue);            TEST_EQUAL(s, "\x1b[34m");
        TRY(s = xt_magenta);         TEST_EQUAL(s, "\x1b[35m");
        TRY(s = xt_cyan);            TEST_EQUAL(s, "\x1b[36m");
        TRY(s = xt_white);           TEST_EQUAL(s, "\x1b[37m");
        TRY(s = xt_black_bg);        TEST_EQUAL(s, "\x1b[40m");
        TRY(s = xt_red_bg);          TEST_EQUAL(s, "\x1b[41m");
        TRY(s = xt_green_bg);        TEST_EQUAL(s, "\x1b[42m");
        TRY(s = xt_yellow_bg);       TEST_EQUAL(s, "\x1b[43m");
        TRY(s = xt_blue_bg);         TEST_EQUAL(s, "\x1b[44m");
        TRY(s = xt_magenta_bg);      TEST_EQUAL(s, "\x1b[45m");
        TRY(s = xt_cyan_bg);         TEST_EQUAL(s, "\x1b[46m");
        TRY(s = xt_white_bg);        TEST_EQUAL(s, "\x1b[47m");
        TRY(s = xt_move_up(42));     TEST_EQUAL(s, "\x1b[42A");
        TRY(s = xt_move_down(42));   TEST_EQUAL(s, "\x1b[42B");
        TRY(s = xt_move_right(42));  TEST_EQUAL(s, "\x1b[42C");
        TRY(s = xt_move_left(42));   TEST_EQUAL(s, "\x1b[42D");
        TRY(s = xt_colour(111));     TEST_EQUAL(s, "\x1b[38;5;16m");
        TRY(s = xt_colour(234));     TEST_EQUAL(s, "\x1b[38;5;67m");
        TRY(s = xt_colour(654));     TEST_EQUAL(s, "\x1b[38;5;223m");
        TRY(s = xt_colour(666));     TEST_EQUAL(s, "\x1b[38;5;231m");
        TRY(s = xt_colour_bg(111));  TEST_EQUAL(s, "\x1b[48;5;16m");
        TRY(s = xt_colour_bg(234));  TEST_EQUAL(s, "\x1b[48;5;67m");
        TRY(s = xt_colour_bg(654));  TEST_EQUAL(s, "\x1b[48;5;223m");
        TRY(s = xt_colour_bg(666));  TEST_EQUAL(s, "\x1b[48;5;231m");
        TRY(s = xt_grey(1));         TEST_EQUAL(s, "\x1b[38;5;232m");
        TRY(s = xt_grey(10));        TEST_EQUAL(s, "\x1b[38;5;241m");
        TRY(s = xt_grey(24));        TEST_EQUAL(s, "\x1b[38;5;255m");
        TRY(s = xt_grey_bg(1));      TEST_EQUAL(s, "\x1b[48;5;232m");
        TRY(s = xt_grey_bg(10));     TEST_EQUAL(s, "\x1b[48;5;241m");
        TRY(s = xt_grey_bg(24));     TEST_EQUAL(s, "\x1b[48;5;255m");

    }

}

TEST_MODULE(core, io) {

    check_file_io_operations();
    check_process_io_operations();
    check_terminal_io_operations();

}
