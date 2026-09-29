// hash_table.h

#pragma once

#include <exception>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <vector>

#ifndef fwd
#define fwd(x) std::forward<decltype(x)>(x)
#endif

/*
    ctor
        init-list [ pair [ k, v ] ]
        size_t buckets
        begin/end iterator [ pair [ k, v ] ],
    dtor
    operator=

    begin/end
    empty
    size
    count
    find
    contains

    at

    insert
    insert_or_assign
    emplace
    try_emplace
    erase
    clear
*/
#include "log.h"

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Cmp = std::equal_to<Key>>
class hash_table {
    // static constexpr uint32_t STATUS_BITS = 0xC0u << 24; // top 2 bits
    // static constexpr uint32_t INDEX_BITS = ~0x00u & ~STATUS_BITS; // bottom 30 bits

    static constexpr uint32_t FREE = 0xf7ee;//0x00ull;
    static constexpr uint32_t BUSY = 0xb15e;//0x40u << 24;
    static constexpr uint32_t TOMBSTONE = 0xdead;//0x80u << 24;
    static constexpr uint32_t MAX_TOMBSTONE_RATIO = 2;
    static constexpr uint32_t MAX_LOAD_RATIO = 2;

    using storage = std::vector<std::pair<Key, Value>>;
    struct index_entry {
        uint32_t status;
        uint32_t index;
        uint32_t hash;
    };

    mutable Hash hasher;
    mutable Cmp cmp;
    
    uint32_t num_buckets;
    uint32_t num_tombstones;
    std::vector<index_entry> index;
    storage nodes;

public:
    using iterator = typename storage::iterator;
    using const_iterator = typename storage::const_iterator;

    hash_table() noexcept: hash_table(8) {}
    explicit hash_table(int buckets) noexcept:
        hasher {},
        cmp {},
        num_buckets { 1u << int(ceil(log2(std::max(buckets, 8)))) },
        index(num_buckets, { FREE, 0xffffffff, 0 }),
        nodes {} {}
    hash_table(hash_table&&) = default;
    hash_table& operator=(hash_table&&) = default;
    hash_table(const hash_table&) = default;
    hash_table& operator=(const hash_table&) = default;
    
    ~hash_table() {}

    void clear() {
        std::fill(index.begin(), index.end(), index_entry { FREE, 0xffffffff, 0 });
        nodes.clear();
    }

    // INTERFACE
    void insert_or_assign(auto&& key, auto&& value) {
        const auto H = hash(fwd(key));
        const auto indexptr = linear_probe(H, fwd(key));
        if (!indexptr) {
            reindex(num_buckets * 2);
            return insert_or_assign(fwd(key), fwd(value));
        }

        auto& indexref = index.at(indexptr - this->index.data()); // HACK:
        if (is_busy(indexref)) {
            nodes.at(indexref.index).second = fwd(value);
        } else {
            nodes.emplace_back(fwd(key), fwd(value));
            indexref = { BUSY, uint32_t(nodes.size() - 1), H };
            if (nodes.size() * MAX_LOAD_RATIO > num_buckets) {
                reindex(num_buckets * 2); // if the load factor is larger than 0.5, eagerly reindex
            }
        }
    }
    
    const_iterator find(auto&& key) const {
        const auto H = hash(fwd(key));
        const auto* indexptr = linear_probe(H, fwd(key));
        if (indexptr && is_busy(*indexptr)) {
            return begin() + indexptr->index;
        }
        return end();
    }
    size_t erase(auto&& key) {
        const auto H = hash(fwd(key));
        const auto indexptr = linear_probe(H, fwd(key));
        if (!indexptr) {
            return 0;
        }
        auto* indexref = &index.at(indexptr - index.data());
        if (is_busy(indexref)) {
            const auto ki = indexref->index;
            const auto back = nodes.size() - 1;
            indexref = { TOMBSTONE, 0xffffffff, 0 };
            
            if (ki != back) {
                // swap the last stored item into this slot
                // log("move from back: ", nodes.back(), "to", ki);
                nodes.at(ki) = std::move(nodes.back());

                // update index entry that was pointing at back
                // const auto H2 = 0u;//hash(nodes.at(ki).first);
                for (auto p = 0u; p < num_buckets; p++) {
                    if (auto& Bn = index.at(bucket(/*H2 + */p)); is_busy(Bn) && Bn.index == back) {
                        Bn.index = ki; break;
                    }
                }
            }

            nodes.pop_back();

            // tombstone "garbage collection"; reindex into a same-size index
            // this keeps the number of tombstones low
            if (num_tombstones * MAX_TOMBSTONE_RATIO > num_buckets) {
                reindex(num_buckets);
            }

            return 1;
        }

        return 0;
    }

    /**/Value DEFAULT;
    const Value& at(auto&& key) const {
        if (const auto node = find(fwd(key)); node != end()) {
            return node->second;
        }
        throw std::runtime_error { "key not found" };
    }

    size_t size() const noexcept { return nodes.size(); }
    bool empty() const noexcept { return nodes.size() == 0; }
    bool contains(auto&& key) const { return find(fwd(key)) != end(); }
    
    iterator begin() noexcept { return nodes.begin(); }
    iterator end() noexcept { return nodes.end(); }
    const_iterator begin() const noexcept { return nodes.begin(); }
    const_iterator end() const noexcept { return nodes.end(); }
    const_iterator cbegin() const noexcept { return nodes.cbegin(); }
    const_iterator cend() const noexcept { return nodes.cend(); }

private:
    // linear probe from H&N, return node matching {H,key} or first free slot
    // tombstones are skipped
    // TODO: we could shuffle tombstones forwards???
    const index_entry* linear_probe(uint32_t H, auto&& key) const noexcept {
        for (auto p = 0u; p < num_buckets; p++) {
            const auto b = bucket(H + p);
            const auto& i = index.at(b);
            if (is_free(i) || (is_busy(i) && i.hash == H && cmp(nodes.at(i.index).first, fwd(key)))) {
                return &i;
            }
        }
        return nullptr;
    }

    void reindex(uint32_t new_num_buckets) {
        auto new_index = std::vector<index_entry>(new_num_buckets, { FREE, 0xffffffff, 0 });
        for (auto& i: index) {
            if (is_busy(i)) {
                for (auto p = 0u; p < new_num_buckets; p++) {
                    const auto b = (i.hash + p) & (new_num_buckets - 1);
                    auto& j = new_index.at(b);
                    if (is_free(j)) {
                        j = i; break;
                    }
                }
            }
        }

        index = std::move(new_index);
        num_buckets = new_num_buckets;
        num_tombstones = 0;
    }

    uint32_t bucket(uint32_t h) const noexcept { return h & (num_buckets - 1); }
    uint32_t hash(auto&& key) const noexcept { return uint32_t(hasher(fwd(key)) & 0xfffffffful); }

    bool is_busy(const index_entry& index) const noexcept { return index.status == BUSY; }
    // bool is_tomb(const index_entry& index) const noexcept { return index.status == TOMBSTONE; }
    bool is_free(const index_entry& index) const noexcept { return index.status == FREE; }
};
