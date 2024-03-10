#pragma once

#include <fstream>
#include <string>
#include <optional>
#include <limits>

std::optional<std::string> read_text_file(const std::string& fname);
bool write_text_file(const std::string& fname, const std::string& data);
template<typename LineCallback> bool read_text_file(const std::string& fname, LineCallback callback) {
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return false;
    }

    auto line = std::string{};
    while (std::getline(file, line)) {
        callback(line);
    }

    return true;
}

#ifdef JLIB_IMPLEMENTATION

std::optional<std::string> read_text_file(const std::string& fname) {
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return std::nullopt;
    }
    //auto start = file.tellg();
    //file.seekg(0, std::ios::end);
    //auto end = file.tellg();
    //auto length = (size_t)(end - start);
    //file.seekg(0, std::ios::beg);

    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = file.gcount();
    file.clear();   //  Since ignore will have set eof.
    file.seekg(0, std::ios_base::beg);    
    
    auto data = std::string(length, 0);
    file.read(data.data(), length);
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