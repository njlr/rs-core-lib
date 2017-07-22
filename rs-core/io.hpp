#pragma once

#include "rs-core/common.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

namespace RS {

    // File I/O operations

    namespace RS_Detail {

        inline bool load_file_helper(FILE* fp, std::string& dst, size_t limit) {
            static constexpr size_t bufsize = 65536;
            size_t offset = 0;
            while (! (feof(fp) || ferror(fp)) && dst.size() < limit) {
                dst.resize(std::min(offset + bufsize, limit), 0);
                offset += fread(&dst[0] + offset, 1, dst.size() - offset, fp);
            }
            dst.resize(offset);
            return ! ferror(fp);
        }

        inline bool save_file_helper(FILE* fp, const void* ptr, size_t n) {
            auto cptr = static_cast<const char*>(ptr);
            size_t offset = 0;
            while (offset < n && ! ferror(fp))
                offset += fwrite(cptr + offset, 1, n - offset, fp);
            return ! ferror(fp);
        }

    }

    #ifdef _XOPEN_SOURCE

        inline bool load_file(const std::string& file, std::string& dst, size_t limit = npos) {
            using namespace RS_Detail;
            dst.clear();
            if (file.empty() || file == "-") {
                return load_file_helper(stdin, dst, limit);
            } else {
                FILE* fp = fopen(file.data(), "rb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return load_file_helper(fp, dst, limit);
            }
        }

        inline bool save_file(const std::string& file, const void* ptr, size_t n, bool append = false) {
            using namespace RS_Detail;
            if (file.empty() || file == "-") {
                return save_file_helper(stdout, ptr, n);
            } else {
                auto fp = fopen(file.data(), append ? "ab" : "wb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return save_file_helper(fp, ptr, n);
            }
        }

    #else

        inline bool load_file(const std::wstring& file, std::string& dst, size_t limit = npos) {
            using namespace RS_Detail;
            dst.clear();
            if (file.empty() || file == L"-") {
                return load_file_helper(stdin, dst, limit);
            } else {
                FILE* fp = _wfopen(file.data(), L"rb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return load_file_helper(fp, dst, limit);
            }
        }

        inline bool save_file(const std::wstring& file, const void* ptr, size_t n, bool append = false) {
            using namespace RS_Detail;
            if (file.empty() || file == L"-") {
                return save_file_helper(stdout, ptr, n);
            } else {
                auto fp = _wfopen(file.data(), append ? L"ab" : L"wb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return save_file_helper(fp, ptr, n);
            }
        }

        inline bool load_file(const std::string& file, std::string& dst, size_t limit = npos) { return load_file(uconv<std::wstring>(file), dst, limit); }
        inline bool save_file(const std::string& file, const void* ptr, size_t n, bool append) { return save_file(uconv<std::wstring>(file), ptr, n, append); }
        inline bool save_file(const std::wstring& file, const std::string& src, bool append = false) { return save_file(file, src.data(), src.size(), append); }

    #endif

    inline bool save_file(const std::string& file, const std::string& src, bool append = false) { return save_file(file, src.data(), src.size(), append); }

    // Process I/O operations

    inline std::string run_command(const U8string& cmd) {
        static constexpr size_t block = 1024;
        std::shared_ptr<FILE> pipe;
        #ifdef _XOPEN_SOURCE
            pipe.reset(popen(cmd.data(), "r"), pclose);
        #else
            auto wcmd = uconv<std::wstring>(cmd);
            pipe.reset(_wpopen(wcmd.data(), L"rb"), pclose);
        #endif
        if (! pipe)
            return {};
        U8string result;
        size_t bytes = 0;
        do {
            size_t offset = result.size();
            result.resize(offset + block);
            bytes = fread(&result[0] + offset, 1, block, pipe.get());
            result.resize(offset + bytes);
        } while (bytes);
        return result;
    }

    // Terminal I/O operations

    inline bool is_stdout_redirected() noexcept { return ! isatty(1); }

    namespace RS_Detail {

        inline int xt_encode_grey(int n) noexcept {
            return 231 + clamp(n, 1, 24);
        }

        inline int xt_encode_rgb(int rgb) noexcept {
            int r = clamp((rgb / 100) % 10, 1, 6);
            int g = clamp((rgb / 10) % 10, 1, 6);
            int b = clamp(rgb % 10, 1, 6);
            return 36 * r + 6 * g + b - 27;
        }

    }

    constexpr const char* xt_up           = "\x1b[A";    // Cursor up
    constexpr const char* xt_down         = "\x1b[B";    // Cursor down
    constexpr const char* xt_right        = "\x1b[C";    // Cursor right
    constexpr const char* xt_left         = "\x1b[D";    // Cursor left
    constexpr const char* xt_erase_left   = "\x1b[1K";   // Erase left
    constexpr const char* xt_erase_right  = "\x1b[K";    // Erase right
    constexpr const char* xt_erase_above  = "\x1b[1J";   // Erase above
    constexpr const char* xt_erase_below  = "\x1b[J";    // Erase below
    constexpr const char* xt_erase_line   = "\x1b[2K";   // Erase line
    constexpr const char* xt_clear        = "\x1b[2J";   // Clear screen
    constexpr const char* xt_reset        = "\x1b[0m";   // Reset attributes
    constexpr const char* xt_bold         = "\x1b[1m";   // Bold
    constexpr const char* xt_under        = "\x1b[4m";   // Underline
    constexpr const char* xt_bold_off     = "\x1b[22m";  // Bold off
    constexpr const char* xt_under_off    = "\x1b[24m";  // Underline off
    constexpr const char* xt_colour_off   = "\x1b[39m";  // Colour off
    constexpr const char* xt_black        = "\x1b[30m";  // Black fg
    constexpr const char* xt_red          = "\x1b[31m";  // Red fg
    constexpr const char* xt_green        = "\x1b[32m";  // Green fg
    constexpr const char* xt_yellow       = "\x1b[33m";  // Yellow fg
    constexpr const char* xt_blue         = "\x1b[34m";  // Blue fg
    constexpr const char* xt_magenta      = "\x1b[35m";  // Magenta fg
    constexpr const char* xt_cyan         = "\x1b[36m";  // Cyan fg
    constexpr const char* xt_white        = "\x1b[37m";  // White fg
    constexpr const char* xt_black_bg     = "\x1b[40m";  // Black bg
    constexpr const char* xt_red_bg       = "\x1b[41m";  // Red bg
    constexpr const char* xt_green_bg     = "\x1b[42m";  // Green bg
    constexpr const char* xt_yellow_bg    = "\x1b[43m";  // Yellow bg
    constexpr const char* xt_blue_bg      = "\x1b[44m";  // Blue bg
    constexpr const char* xt_magenta_bg   = "\x1b[45m";  // Magenta bg
    constexpr const char* xt_cyan_bg      = "\x1b[46m";  // Cyan bg
    constexpr const char* xt_white_bg     = "\x1b[47m";  // White bg

    inline std::string xt_move_up(int n) { return "\x1b[" + RS_Detail::decfmt(n) + 'A'; }                                        // Cursor up n spaces
    inline std::string xt_move_down(int n) { return "\x1b[" + RS_Detail::decfmt(n) + 'B'; }                                      // Cursor down n spaces
    inline std::string xt_move_right(int n) { return "\x1b[" + RS_Detail::decfmt(n) + 'C'; }                                     // Cursor right n spaces
    inline std::string xt_move_left(int n) { return "\x1b[" + RS_Detail::decfmt(n) + 'D'; }                                      // Cursor left n spaces
    inline std::string xt_colour(int rgb) { return "\x1b[38;5;" + RS_Detail::decfmt(RS_Detail::xt_encode_rgb(rgb)) + 'm'; }      // Set fg colour to an RGB value (1-6)
    inline std::string xt_colour_bg(int rgb) { return "\x1b[48;5;" + RS_Detail::decfmt(RS_Detail::xt_encode_rgb(rgb)) + 'm'; }   // Set bg colour to an RGB value (1-6)
    inline std::string xt_grey(int grey) { return "\x1b[38;5;" + RS_Detail::decfmt(RS_Detail::xt_encode_grey(grey)) + 'm'; }     // Set fg colour to a grey level (1-24)
    inline std::string xt_grey_bg(int grey) { return "\x1b[48;5;" + RS_Detail::decfmt(RS_Detail::xt_encode_grey(grey)) + 'm'; }  // Set bg colour to a grey level (1-24)

}
