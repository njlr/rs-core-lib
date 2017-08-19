#include "rs-core/digest.hpp"
#include "rs-core/string.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <numeric>
#include <vector>

using namespace RS;

namespace {

    const U8string sample1 = "";
    const U8string sample2 = "Hello world";
    const U8string sample3 =
        "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad "
        "minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
        "aliquip ex ea commodo consequat. Duis aute irure dolor in "
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
        "culpa qui officia deserunt mollit anim id est laborum.";

    void check_adler32() {

        TEST_EQUAL(uint32_t(Adler32()(sample1.data(), sample1.size())), 0x00000001);
        TEST_EQUAL(uint32_t(Adler32()(sample2.data(), sample2.size())), 0x18ab043d);
        TEST_EQUAL(uint32_t(Adler32()(sample3.data(), sample3.size())), 0x55d5a572);

    }

    void check_crc32() {

        TEST_EQUAL(uint32_t(Crc32()(sample1.data(), sample1.size())), 0x00000000);
        TEST_EQUAL(uint32_t(Crc32()(sample2.data(), sample2.size())), 0x8bd69e52);
        TEST_EQUAL(uint32_t(Crc32()(sample3.data(), sample3.size())), 0x8f92322f);

    }

    constexpr uint64_t sip24_testvec[] = {
        0x726fdb47dd0e0e31ull, 0x74f839c593dc67fdull, 0x0d6c8009d9a94f5aull, 0x85676696d7fb7e2dull, 0xcf2794e0277187b7ull, 0x18765564cd99a68dull, 0xcbc9466e58fee3ceull, 0xab0200f58b01d137ull,
        0x93f5f5799a932462ull, 0x9e0082df0ba9e4b0ull, 0x7a5dbbc594ddb9f3ull, 0xf4b32f46226bada7ull, 0x751e8fbc860ee5fbull, 0x14ea5627c0843d90ull, 0xf723ca908e7af2eeull, 0xa129ca6149be45e5ull,
        0x3f2acc7f57c29bdbull, 0x699ae9f52cbe4794ull, 0x4bc1b3f0968dd39cull, 0xbb6dc91da77961bdull, 0xbed65cf21aa2ee98ull, 0xd0f2cbb02e3b67c7ull, 0x93536795e3a33e88ull, 0xa80c038ccd5ccec8ull,
        0xb8ad50c6f649af94ull, 0xbce192de8a85b8eaull, 0x17d835b85bbb15f3ull, 0x2f2e6163076bcfadull, 0xde4daaaca71dc9a5ull, 0xa6a2506687956571ull, 0xad87a3535c49ef28ull, 0x32d892fad841c342ull,
        0x7127512f72f27cceull, 0xa7f32346f95978e3ull, 0x12e0b01abb051238ull, 0x15e034d40fa197aeull, 0x314dffbe0815a3b4ull, 0x027990f029623981ull, 0xcadcd4e59ef40c4dull, 0x9abfd8766a33735cull,
        0x0e3ea96b5304a7d0ull, 0xad0c42d6fc585992ull, 0x187306c89bc215a9ull, 0xd4a60abcf3792b95ull, 0xf935451de4f21df2ull, 0xa9538f0419755787ull, 0xdb9acddff56ca510ull, 0xd06c98cd5c0975ebull,
        0xe612a3cb9ecba951ull, 0xc766e62cfcadaf96ull, 0xee64435a9752fe72ull, 0xa192d576b245165aull, 0x0a8787bf8ecb74b2ull, 0x81b3e73d20b49b6full, 0x7fa8220ba3b2eceaull, 0x245731c13ca42499ull,
        0xb78dbfaf3a8d83bdull, 0xea1ad565322a1a0bull, 0x60e61c23a3795013ull, 0x6606d7e446282b93ull, 0x6ca4ecb15c5f91e1ull, 0x9f626da15c9625f3ull, 0xe51b38608ef25f57ull, 0x958a324ceb064572ull,
    };

    void check_siphash24() {

        SipHash24 siphash;
        std::vector<uint8_t> key(16), src(64);
        std::iota(key.begin(), key.end(), 0);
        std::iota(src.begin(), src.end(), 0);
        uint64_t result;

        for (size_t srclen = 0; srclen < 64; ++srclen) {
            for (size_t blocklen = 1; blocklen <= srclen; ++blocklen) {
                TRY(siphash = SipHash24(key.data()));
                for (size_t offset = 0; offset < srclen; offset += blocklen)
                    TRY(siphash(src.data() + offset, std::min(srclen - offset, blocklen)));
                TRY(result = siphash);
                TEST_EQUAL(result, sip24_testvec[srclen]);
            }
        }

    }

    void check_md5() {

        using RT = Md5::result_type;

        RT result;
        U8string hex;

        TRY(result = Md5()(sample1.data(), sample1.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "d4 1d 8c d9 8f 00 b2 04 e9 80 09 98 ec f8 42 7e");
        TRY(result = Md5()(sample2.data(), sample2.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "3e 25 96 0a 79 db c6 9b 67 4c d4 ec 67 a7 2c 62");

    }

    void check_sha1() {

        using RT = Sha1::result_type;

        RT result;
        U8string hex;

        TRY(result = Sha1()(sample1.data(), sample1.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "da 39 a3 ee 5e 6b 4b 0d 32 55 bf ef 95 60 18 90 af d8 07 09");
        TRY(result = Sha1()(sample2.data(), sample2.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "7b 50 2c 3a 1f 48 c8 60 9a e2 12 cd fb 63 9d ee 39 67 3f 5e");

    }

    void check_sha256() {

        using RT = Sha256::result_type;

        RT result;
        U8string hex;

        TRY(result = Sha256()(sample1.data(), sample1.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "e3 b0 c4 42 98 fc 1c 14 9a fb f4 c8 99 6f b9 24 "
                        "27 ae 41 e4 64 9b 93 4c a4 95 99 1b 78 52 b8 55");
        TRY(result = Sha256()(sample2.data(), sample2.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "64 ec 88 ca 00 b2 68 e5 ba 1a 35 67 8a 1b 53 16 "
                        "d2 12 f4 f3 66 b2 47 72 32 53 4a 8a ec a3 7f 3c");

    }

    void check_sha512() {

        using RT = Sha512::result_type;

        RT result;
        U8string hex;

        TRY(result = Sha512()(sample1.data(), sample1.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "cf 83 e1 35 7e ef b8 bd f1 54 28 50 d6 6d 80 07 "
                        "d6 20 e4 05 0b 57 15 dc 83 f4 a9 21 d3 6c e9 ce "
                        "47 d0 d1 3c 5d 85 f2 b0 ff 83 18 d2 87 7e ec 2f "
                        "63 b9 31 bd 47 41 7a 81 a5 38 32 7a f9 27 da 3e");
        TRY(result = Sha512()(sample2.data(), sample2.size()));
        TRY(hex = hexdump(result.data(), result.size()));
        TEST_EQUAL(hex, "b7 f7 83 ba ed 82 97 f0 db 91 74 62 18 4f f4 f0 "
                        "8e 69 c2 d5 e5 f7 9a 94 26 00 f9 72 5f 58 ce 1f "
                        "29 c1 81 39 bf 80 b0 6c 0f ff 2b dd 34 73 84 52 "
                        "ec f4 0c 48 8c 22 a7 e3 d8 0c df 6f 9c 1c 0d 47");

    }

}

TEST_MODULE(core, digest) {

    check_adler32();
    check_crc32();
    check_siphash24();
    check_md5();
    check_sha1();
    check_sha256();
    check_sha512();

}
