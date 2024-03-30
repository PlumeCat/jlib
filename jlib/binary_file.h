#pragma once

#include <vector>

// read binary file
// blockingly read the entire file in one go
// returns false if file couldn't be opened or read; true otherwise
// out_data is overwritten and resized once to fit the entire file
bool read_binary_file(const std::string& fname, std::vector<uint8_t>& out_data);

// write binary file
// blockingly write the whole file in one go
// returns true if all data was successfully written; false otherwise
bool write_binary_file(const std::string& fname, const std::vector<uint8_t>& data);

#ifdef JLIB_IMPLEMENTATION

#include <fstream>
#include <filesystem>

bool read_binary_file(const std::string& fname, std::vector<uint8_t>& out_data) {
    if (!std::filesystem::exists(fname)) {
        return false;
    }
    auto length = std::filesystem::file_size(fname);
    auto file = std::ifstream(fname);
    if (!file.is_open()) {
        return false;
    }

    out_data.resize(length);
    file.read((char*)out_data.data(), length);
    return true;
}

bool write_binary_file(const std::string& fname, const std::vector<uint8_t>& data) {
    auto file = std::ofstream(fname, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write((const char*)data.data(), data.size());
    // TODO: check badbit/n written
    return true;
}

#endif