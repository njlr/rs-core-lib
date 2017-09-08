#pragma once

#include "rs-core/common.hpp"
#include <algorithm>
#include <cstdio>
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

}
