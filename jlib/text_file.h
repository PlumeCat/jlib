#pragma once

#include <fstream>
#include <string>
#include <optional>

std::optional<std::string> read_text_file(const std::string& fname);
bool write_text_file(const std::string& fname, const std::string& data);

#ifdef JLIB_IMPLEMENTATION

std::optional<std::string> read_text_file(const std::string& fname) {
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return std::nullopt;
    }
    auto start = file.tellg();
    file.seekg(0, std::ios::end);
    auto end = file.tellg();
    auto size = (size_t)(end - start);
    file.seekg(0, std::ios::beg);
    auto data = std::string(size, 0);
    file.read(data.data(), size);
    return std::optional(data);
}

bool write_text_file(const std::string& fname, const std::string& data) {
    auto file = std::ofstream(fname);
    if (!file.is_open()) {
        return false;
    }
    file.write(data.data(), data.size());
    return true;
}


#endif