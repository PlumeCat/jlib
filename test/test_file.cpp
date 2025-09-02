#include <filesystem>
#include <jlib/binary_file.h>
#include <jlib/test_framework.h>
#include <jlib/text_file.h>

const auto TMPDIR = std::filesystem::temp_directory_path();
const auto TEXTFILE = TMPDIR / "jlib_test_textfile.txt";
const auto BINFILE  = TMPDIR / "jlib_test_binfile.dat";

TEST("write text file") {
    auto data = "hello world\nthis is a c++ program\nend";
    ASSERT(write_text_file(TEXTFILE, data));
}

TEST("read text file") {
    auto data = std::string {};
    ASSERT(read_text_file(TEXTFILE, data));
    ASSERT(!read_text_file("SHOULD_NOT_EXIST", data));
}

TEST("write binary file") {
    auto data = std::vector<uint8_t> { 1, 2, 3 };
    ASSERT(write_binary_file(BINFILE, data));
    ASSERT(std::filesystem::file_size(BINFILE) == 3);
}

TEST("read binary file") {
    auto data = std::vector<uint8_t> {};
    ASSERT(read_binary_file(BINFILE, data));
    ASSERT(!read_binary_file("NOT EXIST", data));
    ASSERT(data == std::vector<uint8_t> { 1, 2, 3 });
}

TEST("read text file callback") {
    auto data = std::vector<std::string> {};
    ASSERT(read_text_file(TEXTFILE, [&](const std::string& line) -> bool {
        data.emplace_back(line);
        return true;
    }));

    ASSERT(data == decltype(data) { "hello world", "this is a c++ program", "end" });
}

