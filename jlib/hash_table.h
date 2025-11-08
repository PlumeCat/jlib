// hash_table.h

#pragma once

#include <exception>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <memory>
#include <vector>

#ifndef FORWARD
#define FORWARD(x) std::forward<decltype(x)>(x)
#endif

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Cmp = std::equal_to<Key>> class hash_table {
    static constexpr uint32_t STATUS_BITS = 0xC0u << 24; // top 2 bits
    static constexpr uint32_t INDEX_BITS = ~0x00u & ~STATUS_BITS; // bottom 30 bits

    static constexpr uint32_t FREE = 0x00ull;
    static constexpr uint32_t TOMBSTONE = 0x80u << 24;
    static constexpr uint32_t BUSY = 0x40u << 24;

    using Storage = std::vector<std::pair<Key, Value>>;
    struct Index {
        uint32_t s_ind; // status:2 | index:30
        uint32_t hash;
    };

    mutable Hash hasher;
    mutable Cmp cmp;
    uint32_t num_buckets;
    std::unique_ptr<Index[]> index;
    Storage nodes;

public:

    using iterator = typename Storage::iterator;
    using const_iterator = typename Storage::const_iterator;

    hash_table() noexcept:
        hash_table(8) {}
    explicit hash_table(int buckets) noexcept:
        hasher {},
        cmp {},
        num_buckets { 1u << int(ceil(log2(std::max(8, buckets)))) },
        index { std::make_unique<Index[]>(num_buckets) },
        nodes {} {}
    hash_table(hash_table&&) = default;
    hash_table& operator=(hash_table&&) = default;
    hash_table(const hash_table&) = default;
    hash_table& operator=(const hash_table&) = default;
    ~hash_table() {}

    // INTERFACE
    void insert_or_assign(auto&& key, auto&& value) {
        const auto h = hash(FORWARD(key));
        auto* indexptr = linear_probe(h, FORWARD(key));
        if (!indexptr) {
            reindex();
            insert_or_assign(FORWARD(key), FORWARD(value));
            return;
        }

        const auto i = *indexptr;
        if (is_busy(i)) {
            // assign case
            nodes[i.s_ind & INDEX_BITS].second = FORWARD(value);
        } else {
            // inserted case
            const auto n = (uint32_t)nodes.size();
            *indexptr = { BUSY | (n & INDEX_BITS), h };
            nodes.emplace_back(FORWARD(key), FORWARD(value));

            // if the load factor is larger than 0.5, eagerly reindex
            if (nodes.size() > num_buckets / 2) {
                reindex();
            }
        }
    }
    auto find(auto&& key) const { return get(FORWARD(key)); }
    const_iterator get(auto&& key) const {
        const auto h = hash(FORWARD(key));
        const auto* indexptr = linear_probe(h, FORWARD(key));
        if (!indexptr) {
            return cend();
        }
        return cbegin() + (indexptr->s_ind & INDEX_BITS);
    }
    void erase(auto&& key) {
        const auto h = hash(FORWARD(key));
        auto* indexptr = linear_probe(h, FORWARD(key));
        if (!indexptr) {
            return;
        }

        const auto i = *indexptr;
        if (is_busy(i)) {
            const auto ind = i.s_ind & INDEX_BITS;
            const auto back = nodes.size() - 1;
            indexptr->s_ind = TOMBSTONE;
            if (ind != back) {
                std::swap(nodes[ind], nodes[back]);
                const auto h = hash(nodes[ind].first);
                for (auto p = 0; p < num_buckets; p++) {
                    const auto b = bucket(h + p);
                    auto& i = index[b];
                    if ((i.s_ind & INDEX_BITS) == back) {
                        i.s_ind = BUSY | (ind & INDEX_BITS);
                        break;
                    }
                }
            }
            nodes.resize(nodes.size() - 1);
        }
    }

    iterator begin() noexcept { return nodes.begin(); }
    iterator end() noexcept { return nodes.end(); }
    const_iterator begin() const noexcept { return nodes.begin(); }
    const_iterator end() const noexcept { return nodes.end(); }
    const_iterator cbegin() const noexcept { return nodes.cbegin(); }
    const_iterator cend() const noexcept { return nodes.cend(); }

    auto size() const { return nodes.size(); }

    bool contains(auto&& key) const {
        return get(FORWARD(key)) != end();
    }
    const Value& at(auto&& key) const {
        if (auto node = get(FORWARD(key)); node != end()) {
            return node->second;
        }
        throw std::exception {};
    }

private:
    Index* linear_probe(uint32_t h, auto&& key) const {
        for (auto p = 0; p < num_buckets; p++) {
            const auto b = bucket(h + p);
            auto& i = index[b];
            if (is_free(i) || (i.hash == h && cmp(nodes[i.s_ind & INDEX_BITS].first, FORWARD(key)))) {
                return &i;
            }
        }
        return nullptr;
    }

    void reindex() {
        const auto new_num_buckets = num_buckets * 2;
        auto new_index = std::make_unique<Index[]>(new_num_buckets);

        for (auto _ = 0; _ < num_buckets; _++) {
            auto& i = index[_];
            if (is_busy(i)) {
                for (auto p = 0; p < new_num_buckets; p++) {
                    const auto b = (i.hash + p) & (new_num_buckets - 1);
                    auto& j = new_index[b];
                    if (is_free(j)) {
                        j = i;
                        break;
                    }
                }
            }
        }

        index = std::move(new_index);
        num_buckets = new_num_buckets;
    }

    uint32_t bucket(uint32_t h) const noexcept { return h & (num_buckets - 1); }
    uint32_t hash(auto&& key) const noexcept { return uint32_t(hasher(FORWARD(key)) & 0xfffffffful); }

    bool is_busy(const Index& index) const noexcept { return index.s_ind & BUSY; }
    bool is_tomb(const Index& index) const noexcept { return index.s_ind & TOMBSTONE; }
    bool is_free(const Index& index) const noexcept { return (index.s_ind & STATUS_BITS) == FREE; }
};
