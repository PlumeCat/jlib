#pragma once

#include <string>
#include <fstream>
#include <filesystem>

// LineCallback signature: bool (*)(const std::string&)
template<typename T> concept LineCallback = requires (T t) { { t(std::string{}) } -> std::same_as<bool>; };

// read text file
// blockingly read the entire file in one go
// returns false if file couldn't be opened or read; true otherwise
// out_data is overwritten and resized once to fit the entire file
bool read_text_file(const std::string& fname, std::string& out_data);

// read text file
// read sequentially line by line and call the callback once for each line
// returns false if file couldn't be opened or read; true otherwise
// callback should have signature bool(*)(const std::string&);
// return false from the callback to stop reading any more lines
bool read_text_file(const std::string& fname, LineCallback auto callback) {
    if (!std::filesystem::exists(fname)) {
        return false;
    }
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return false;
    }

    auto line = std::string {};
    while (std::getline(file, line) && callback(line));
    return true;
}

// write text file
// blockingly write the whole file in one go
// returns true if all data was successfully written; false otherwise
bool write_text_file(const std::string& fname, const std::string& data);

#ifdef JLIB_IMPLEMENTATION

bool read_text_file(const std::string& fname, std::string& out_data) {
    if (!std::filesystem::exists(fname)) {
        return false;
    }
    auto length = std::filesystem::file_size(fname);
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return false;
    }

    out_data.resize(length);
    file.read(out_data.data(), length);
    return true;
}

bool write_text_file(const std::string& fname, const std::string& data) {
    auto file = std::ofstream(fname);
    if (!file.is_open()) {
        return false;
    }
    file.write(data.data(), data.size());
    // TODO: check badbit
    return true;
}

#endif