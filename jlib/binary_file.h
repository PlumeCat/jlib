#pragma once
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>

// read binary file
// blockingly read the entire file in one go
// returns false if file couldn't be opened or read; true otherwise
// out_data is overwritten and resized once to fit the entire file
// bool read_binary_file(const std::filesystem::path& path, std::vector<uint8_t>& out_data);
template<typename T = uint8_t> bool read_binary_file(const std::filesystem::path& path, std::vector<T>& out_data) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    auto length = std::filesystem::file_size(path);
    auto file = std::ifstream(path);
    if (!file.is_open()) {
        return false;
    }

    out_data.resize(length / sizeof(T) + ((length % sizeof(T)) ? 1 : 0));
    file.read(reinterpret_cast<char*>(out_data.data()), length);
    return true;
}

// write binary file
// blockingly write the whole file in one go
// returns true if all data was successfully written; false otherwise
bool write_binary_file(const std::filesystem::path& path, const std::vector<uint8_t>& data);

#ifdef JLIB_IMPLEMENTATION

bool write_binary_file(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
    auto file = std::ofstream(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    // TODO: check badbit/n written
    return true;
}

#endif
