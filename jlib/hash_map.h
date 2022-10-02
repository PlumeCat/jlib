// hash_map.h

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <algorithm>

// template<typename Key, typename Value>
// struct Key{}; struct Value{};
using Key = std::string;
using Value = std::string;

// linear probed
struct hash_map {
    struct Node {
        bool alive;
        size_t hash;
        Key key;
        Value value;
    };

    std::unique_ptr<Node[]> data;
    size_t capacity;
    size_t count;

    std::hash<Key> hasher;

public:
    hash_map(const hash_map&) = default;
    hash_map(hash_map&&) = default;
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&&) = default;

    hash_map(size_t initial_capacity, float max_load_factor):
        capacity(initial_capacity),
        count(0),
        data(std::make_unique<Node[]>(initial_capacity)) {
        for (auto i = 0; i < capacity; i++) {
            data.get()[i].alive = false;
        }
    }

    Value get(const Key& key) {
        // todo
        auto pos = get_pos(key);
        return data.get()[pos].value;
    }
    void erase(const Key& key) {
        // todo
    }
    void set(const Key& key, const Value& value) {
        // todo
    }
    void clear() {
        // todo
    }

private:
    size_t get_hash(const Key& key) {
        return hasher(key);
    }
    size_t get_pos(const Key& key) {
        auto hash = get_hash(key);
        auto index = hash % capacity;
        auto ptr = data.get();
        while (ptr[index].alive) {
            index++;
        }
        return index;
    }
};
