#include <jlib/test_framework.h>
#include <jlib/binary_file.h>
#include <jlib/text_file.h>

TEST("write text file") {
    auto data = "hello world\nthis is a c++ program\nend";
    ASSERT(write_text_file("test/test_data.txt", data));
}

TEST("read text file") {
    auto data = std::string {};
    ASSERT(read_text_file("test/test_data.txt", data));
    ASSERT(!read_text_file("NOT_EXIST", data));
}

TEST("write binary file") {
    auto data = std::vector<uint8_t> { 1, 2, 3 };
    ASSERT(write_binary_file("test/foo.data", data));
    ASSERT(std::filesystem::file_size("test/foo.data") == 3);
}

TEST("read binary file") {
    auto data = std::vector<uint8_t>{};
    ASSERT(read_binary_file("test/foo.data", data));
    ASSERT(!read_binary_file("NOT EXIST", data));
    ASSERT(data == std::vector<uint8_t> { 1, 2, 3 });
}

TEST("read text file callback") {
    auto data = std::vector<std::string> {};
    ASSERT(read_text_file("test/test_data.txt", [&] (const std::string& line) -> bool {
        data.emplace_back(line);
        return true;
    }));

    ASSERT(data == decltype(data) {
        "hello world",
        "this is a c++ program",
        "end"
    });
}