// hash_map2
#pragma once

#include <algorithm>
#include <exception>
#include <variant>
#include <vector>
#include <cmath>

#pragma once

template<typename Key, typename Value, typename Hash=std::hash<Key>>
class hashmap {
protected:
    using KeyType = Key;
    using ValueType = Value;
    using HashType = decltype(Hash{}(std::declval<Key>()));

    enum class Flags {
        Free,
        Busy,
        Tombstone
    };
    struct Node {
        Flags flags;
        HashType hash;
        Key key;
        Value value;

        Node(): flags(Flags::Free), hash(0), key(), value() {}
        Node(Flags flags, HashType hash, const Key& key, const Value& value)
            : flags(flags), hash(hash), key(key), value(value) {}
        Node(Node&&) = default;
        Node(const Node&) = delete;
        Node& operator=(Node&&) = default;
        Node& operator=(const Node&) = delete;
    };

    Hash hash;
    static const int salt = 0; // prime
    int count = 0;
    int num_buckets;


    std::vector<Node> nodes;

public:
    hashmap() noexcept: hashmap(8) {}
    explicit hashmap(int buckets) noexcept:
        count(0),
        num_buckets(1 << int(ceil(log2(std::max(8, buckets))))),
        nodes(num_buckets) {}
    hashmap(const hashmap& h) = default;
    hashmap& operator=(const hashmap& h) = default;
    hashmap(hashmap&& h) = default;
    hashmap& operator=(hashmap&& h) = default;
    ~hashmap() = default;

    auto begin() { return nodes.begin(); }

    void insert_or_assign(const Key& key, const Value& value) noexcept {
        // hash
        auto h = hash(key);

        auto* node = linear_probe(h, key);
        if (!node) {
            grow(); // didn't find a free bucket; grow and try again
            insert_or_assign(key, value);
            return;
        }

        // free node? inserting key for the first time, set key and hash
        auto& n = nodes[node - &nodes[0]]; // hmm
        if (n.flags == Flags::Free) {
            n.hash = h;
            n.key = key;
            count++;
        }

        // always set flags and value
        n.flags = Flags::Busy;
        n.value = value;
    }
    const Node* get(const Key& key) const noexcept {
        // hash
        auto h = hash(key);
        auto* node = linear_probe(h, key);
        if (node && node->flags == Flags::Busy) {
            return node;
        }
        return nullptr;
    }

    int size() const noexcept { return count; }
    float load_factor() const noexcept { return (float)count / num_buckets; }

    bool contains(const Key& key) const noexcept {
        return get(key) != nullptr;
    }
    const Value& at(const Key& key) const {
        if (const auto* node = get(key)) {
            return node->value;
        }
        throw std::exception {};
    }
    void erase(const Key& key) noexcept {
        if (const auto* node = get(key)) {
            nodes[node - &nodes[0]].flags = Flags::Tombstone; // hmm
            count--;
        }
    }
    void clear() noexcept {
        count = 0;
        for (auto i = 0; i < num_buckets; i++) {
            nodes[i].flags = Flags::Free;
        }
    }

protected:
    int bucket(HashType h) const noexcept {
        return (h + salt) & (num_buckets - 1);
    }

    // returns first matching node
    // if node is busy or tombstone, h/key will match
    // else node is free
    const Node* linear_probe(HashType h, const Key& key) const {
        for (auto p = 0; p < num_buckets; p++) {
            auto b = bucket(h + p);
            auto& node = nodes[b];
            if (node.flags == Flags::Free) {
                return &node; // found free node -> key was never inserted
            }
            if (node.hash == h && node.key == key) {
                return &node; // found node with matching key
            }
        }

        return nullptr;
    }

    void grow() noexcept {
        // TODO: statistically unlikely, but this is technically unbounded
        // could loop forever eating exponential memory
        auto x = hashmap(num_buckets * 2);
        for (auto& node: nodes) {
            if (node.flags == Flags::Busy) {
                x.insert_or_assign(node.key, node.value);
            }
        }
        *this = std::move(x);
    }
};
