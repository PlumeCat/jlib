// hash_map
#pragma once

#include <algorithm>
#include <exception>
#include <variant>
#include <vector>
#include <cmath>
#include <memory>

#pragma once

#define FORWARD(x) std::forward<decltype(x)>(x)

template<typename Key, typename Value, typename Hash=std::hash<Key>, typename Cmp=std::equal_to<Key>>
class hash_map {
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
        Key first; // HACK:
        Value second; // HACK:

        Node(): flags(Flags::Free), hash(0), first(), second() {}
        Node(Flags flags, HashType hash, auto&& key, auto&& value)
            : flags(flags), hash(hash), first(FORWARD(key)), second(FORWARD(value)) {}
        Node(Node&&) = default;
        Node(const Node&) = delete;
        Node& operator=(Node&&) = default;
        Node& operator=(const Node&) = delete;
    };

    Hash hash;
    Cmp cmp;
    static const int salt = 0; // prime
    int count = 0;
    int num_buckets;

    std::unique_ptr<Node[]> nodes;

public:
    hash_map() noexcept: hash_map(8) {}
    explicit hash_map(int buckets) noexcept:
        count(0),
        num_buckets(1 << int(ceil(log2(std::max(8, buckets))))),
        nodes(std::make_unique<Node[]>(num_buckets)) {}
    hash_map(const hash_map& h) = default;
    hash_map& operator=(const hash_map& h) = default;
    hash_map(hash_map&& h) = default;
    hash_map& operator=(hash_map&& h) = default;
    ~hash_map() {}

    auto begin() const { return nodes.get(); }

    // void insert_or_assign(const Key& key, const Value& value) noexcept {
    void insert_or_assign(auto&& key, auto&& value) noexcept {
        // hash
        auto h = hash(key);

        auto* node = linear_probe(h, FORWARD(key));
        if (!node) {
            grow(); // didn't find a free bucket; grow and try again
            insert_or_assign(FORWARD(key), FORWARD(value));
            return;
        }

        // free node? inserting key for the first time, set key and hash
        auto& n = nodes[node - &nodes[0]]; // hmm
        if (n.flags == Flags::Free) {
            n.hash = h;
            n.first = FORWARD(key);
            count++;
        }

        // always set flags and value
        n.flags = Flags::Busy;
        n.second = FORWARD(value);
    }
    // const Node* get(const Key& key) const noexcept {
    const Node* get(auto&& key) const noexcept {
        // hash
        auto h = hash(key);
        auto* node = linear_probe(h, FORWARD(key));
        if (node && node->flags == Flags::Busy) {
            return node;
        }
        return nullptr;
    }

    int size() const noexcept { return count; }
    float load_factor() const noexcept { return (float)count / num_buckets; }

    bool contains(auto&& key) const noexcept {
        return get(FORWARD(key)) != nullptr;
    }
    // const Value& at(const Key& key) const {
    const Value& at(auto&& key) const {
        if (const auto* node = get(FORWARD(key))) {
            return node->second;
        }
        throw std::exception {};
    }

    // void erase(const Key& key) noexcept {
    void erase(auto&& key) noexcept {
        if (const auto* node = get(FORWARD(key))) {
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
        return (h) & (num_buckets - 1);
    }

    // returns first matching node
    // if node is busy or tombstone, hash and key will match
    // else node is free
    const Node* linear_probe(HashType h, auto&& key) const {
        for (auto p = 0; p < num_buckets; p++) {
            auto b = bucket(h + p);
            auto& node = nodes[b];
            if (node.flags == Flags::Free) {
                return &node; // found free node -> key was never inserted
            }
            if (node.hash == h && cmp(node.first, FORWARD(key))) {
                return &node; // found node with matching key
            }
        }

        return nullptr;
    }

    void grow() noexcept {
        // TODO: statistically unlikely, but this is technically unbounded
        // could loop forever eating exponential memory
        auto x = hash_map(num_buckets * 2);
        for (auto p = 0; p < num_buckets; p++) {
            const auto& node = nodes[p];
            if (node.flags == Flags::Busy) {
                x.insert_or_assign(std::move(node.first), std::move(node.second));
            }
        }
        *this = std::move(x);
    }
};


#undef FORWARD
