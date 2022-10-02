// hash_map.h

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

template<typename Key, typename Value>
struct hash_map {
    struct Node {
        bool alive = false;
        bool tombstone;
        size_t hash;
        Key key;
        Value value;
    };

    std::unique_ptr<Node[]> data;
    size_t capacity;
    size_t count;

public:
    hash_map(const hash_map&) = default;
    hash_map(hash_map&&) = default;
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&&) = default;

    hash_map(size_t initial_size, float max_load_factor) {
        // todo
        data = std::make_unique<Node[]>();
    }

    Value get(const Key& key) {
        // todo
        auto pos = get_pos();
        return data.get()[pos];
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
        return std::hash(key);
    }
    size_t get_pos(const Key& key) {
        auto hash = get_hash(key);
        auto index = hash % capacity;
        if (data.get()[index]) {

        }
        return 0;
    }
};
