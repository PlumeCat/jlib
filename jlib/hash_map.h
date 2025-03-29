// hash_map.h
#pragma once

#include <cstdint>
#include <exception>
#include <limits>
#include <string>
#include <vector>
#include <stdexcept>

#include "common.h"

// TODO: must never return EMPTY (currently can't use 1<<32 as a key)
// TODO: or replace EMPTY sentinels with a dedicated value
// TODO: maybe u64 or size_t instead of u32
struct default_hash {
    uint32_t operator()(const IsEnum auto& t) const {
        return (uint32_t)t;
    }
    uint32_t operator()(const std::integral auto& t) const {
        return uint32_t{t};
    }
    uint32_t operator()(const std::string& t) const {
        const uint64_t p = 31; // 53
        const uint64_t m = 1e9 + 9;
        uint64_t hash = 0;
        uint64_t pp = 1;
        for (auto c: t) {
            // TODO: % slow, find something that uses XOR
            hash = (hash + (c - 'a' + 1) * pp) % m;
            pp = (pp * p) % m; // TODO: can overflow (is consistent though)
        }
        return hash;
    }
};


// simple open address hash map
// there are N slots for a given hash collision
// if they are exhausted, capacity is doubled (bucket size stays the same) and rehash everything
// TODO: replace with hopefully better performing "shift forward hash map"


template<typename Key, typename Value, typename HashFunc = default_hash>
class hash_map {

    template <typename node_iter, typename pair_type> struct iter {
        node_iter i, N;

        iter(node_iter i, node_iter n): i(i), N(n) {
            if (i < N && i->second == EMPTY) { operator++(); }
        }
        pair_type& operator*() { return i->first; }
        pair_type* operator->() { return &i->first; }
        bool operator != (const iter& _) const { return i != _.i; }
        bool operator == (const iter& _) const { return i == _.i; }

        iter& operator++() {
            if (i == N) { return *this; }
            do i++; while (i < N && i->second == EMPTY);
            return *this;
        }
        iter operator++(int) {
            auto _this = *this;
            operator++();
            return _this;
        }
    };

public:
    using type = hash_map<Key, Value, HashFunc>;
    using pair = std::pair<Key, Value>;
    struct node {
        pair data;
        size_t hash;
        uint8_t state;
    };
    using storage = std::vector<node>;

    explicit hash_map(uint32_t num_buckets = 4):
        N(num_buckets),
        hash_func(HashFunc{}),
        nodes(num_buckets, node { pair { Key(), Value() }, EMPTY }),
        num_nodes(0u) {}

    hash_map(std::initializer_list<pair>& initializer): hash_map() {
        for (auto& [ k, v ]: initializer) {
            emplace(k, v);
        }
    }

    hash_map(const hash_map&) = default;
    hash_map(hash_map&& other):
        N(other.N),
        num_nodes(other.num_nodes),
        nodes(std::move(other.nodes)),
        hash_func(other.hash_func) {
        other.clear();
    }
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&& other) {
        if (this != &other) {
            N = other.N;
            num_nodes = other.num_nodes;
            nodes = std::move(other.nodes);
            hash_func = other.hash_func;
            other.clear();
        }
        return *this;
    }



    // Iterators
    iterator begin() { return iterator { nodes.begin(), nodes.end() }; }
    iterator end() { return iterator { nodes.end(), nodes.end() }; }
    const_iterator begin() const { return const_iterator { nodes.begin(), nodes.end() }; }
    const_iterator end() const { return const_iterator { nodes.end(), nodes.end() }; }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

private:
    size_t N;
    storage nodes;
    size_t num_nodes;
    HashFunc hash_func;

};



// should have good random access performance because guarantees max $bucket_size comparisons
// however higher probability of wasting more memory due to overeager hashing (no linear probe)
template <typename Key, typename Value, typename HashFunc = default_hash, uint32_t EMPTY = std::numeric_limits<uint32_t>::max()>
class hash_map2 {
    template <typename node_iter, typename pair_type> struct iter {
        node_iter i, N;

        iter(node_iter i, node_iter n): i(i), N(n) {
            if (i < N && i->second == EMPTY) { operator++(); }
        }
        pair_type& operator*() { return i->first; }
        pair_type* operator->() { return &i->first; }
        bool operator != (const iter& _) const { return i != _.i; }
        bool operator == (const iter& _) const { return i == _.i; }

        iter& operator++() {
            if (i == N) { return *this; }
            do i++; while (i < N && i->second == EMPTY);
            return *this;
        }
        iter operator++(int) {
            auto _this = *this;
            operator++();
            return _this;
        }
    };


public:
    using type = hash_map<Key, Value, HashFunc, EMPTY>;
    using kv_pair = std::pair<Key, Value>;
    using node = std::pair<kv_pair, uint32_t>; // key, value, hash
    using storage = std::vector<node>;
    // using iterator = iter<hash_map, kv_pair>;
    // using const_iterator = iter<const hash_map, const kv_pair>;
    using iterator = iter<typename storage::iterator, kv_pair>;
    using const_iterator = iter<typename storage::const_iterator, const kv_pair>;

    explicit hash_map2(uint32_t num_buckets = 2, uint32_t bucket_size = 2):
        N(num_buckets),
        K(bucket_size),
        NK(num_buckets * bucket_size),
        hash_func(HashFunc {}),
        nodes(num_buckets * bucket_size, node { kv_pair { Key(), Value() }, EMPTY }),
        num_nodes(0u) {}
    hash_map2(const std::initializer_list<kv_pair>& initializer): hash_map2() {
        for (auto& [k, v]: initializer) {
            emplace(k, v);
        }
    }
    hash_map2(const hash_map2&) = default;
    hash_map2(hash_map2&& other):
        N(other.N),
        K(other.K),
        NK(other.NK),
        hash_func(other.hash_func),
        nodes(std::move(other.nodes)),
        num_nodes(other.num_nodes) {
        other = hash_map2 {};
    }

    hash_map2& operator=(const hash_map2&) = default;
    hash_map2& operator=(hash_map2&& other) {
        if (this != &other) {
            N = other.N; K = other.K; NK = other.NK;
            nodes = std::move(other.nodes);
            num_nodes = other.num_nodes;
            hash_func = other.hash_func;
            other = hash_map2 {};
        }
        return *this;
    }

    // Iterators
    iterator begin() { return iterator { nodes.begin(), nodes.end() }; }
    iterator end() { return iterator { nodes.end(), nodes.end() }; }
    const_iterator begin() const { return const_iterator { nodes.begin(), nodes.end() }; }
    const_iterator end() const { return const_iterator { nodes.end(), nodes.end() }; }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    // number of unique inserts, subtract number of valid removes
    size_t size() const { return num_nodes; }

    // size equal to 0
    bool empty() const { return size() == 0; }

    // remove everything
    void clear() {
        N = 2; K = 2;
        NK = N * K;
        nodes.assign(NK, node { { Key(), Value() }, EMPTY });
        num_nodes = 0;
    }

    // Find/contains
    bool contains(const Key& key) const {
        return lookup(key, hash_func(key)).second;
    }
    iterator find(const Key& key) {
        auto [ index, exist ] = lookup(key, hash_func(key));
        if (exist) { return iterator { nodes.begin(), nodes.end() }; }
        return end();
    }
    const_iterator find(const Key& key) const {
        auto [ index, exist ] = lookup(key, hash_func(key));
        if (exist) { return iterator { nodes.begin(), nodes.end() }; }
        return end();
    }

    // index operator
    Value& operator[](const Key& key) {
        return emplace(key, Value()).first->second;
    }

    // get value for the given key
    // throws exception if key not found
    const Value& at(const Key& key) const {
        auto [ index, exist ] = lookup(key, hash_func(key));
        if (!exist) {
            throw std::runtime_error("Key not found:");
        }
        return nodes[index].first.second;
    }

    // insert a key-value
    // if the key was already present, does nothing
    // returns iterator to new or existing element, bool indicating whether inserted
    std::pair<iterator, bool> emplace(const Key& key, const Value& value) {
        const auto hash = hash_func(key);
        const auto [ index, exist ] = lookup(key, hash);

        // already existed
        if (exist) {
            return { iterator { nodes.begin() + index, nodes.end() }, false };
        }

        // didn't exist, no space in bucket
        if (index == EMPTY) {
            resize();
            return emplace(key, value);
        }

        // didn't exist, space available to insert
        nodes[index] = node { { key, value }, hash };
        num_nodes++;
        return { iterator { nodes.begin() + index, nodes.end() }, true };
    }
    size_t erase(const Key& key) {
        const auto [ index, exist ] = lookup(key, hash_func(key));
        if (!exist) { return 0; }
        num_nodes--;
        nodes[index] = node { { Key(), Value() }, EMPTY };
        return 1;
    }

private:
    void resize() {
        auto new_hash_map = type(N * 2, K);
        for (auto& n: nodes) {
            if (n.second != EMPTY) {
                new_hash_map.emplace(n.first.first, n.first.second);
            }
        }
        *this = std::move(new_hash_map);
    }

    // if the element exists, return { index, true }
    // if the element does not exist, return { pos, false }
    //      where pos is the position to insert
    //      or EMPTY if no valid positions (insert needs to resize)
    std::pair<uint32_t, bool> lookup(const Key& key, uint32_t hash) const {
        const auto bucket_index = hash & (N - 1);
        const auto bucket_pos = bucket_index * K;
        auto key_pos = EMPTY; // the pos where this key is or could be
        for (auto i = 0u; i < K; i++) {
            const auto h = nodes[bucket_pos + i].second; // hash at current slot
            if (h == EMPTY) {
                // slot i is empty, key *could* go here if location not already found
                // note: don't break yet, in case key exists further down the bucket
                key_pos = (key_pos == EMPTY) ? bucket_pos + i : key_pos;
            } else if (h == hash && nodes[bucket_pos + i].first.first == key) {
                // key already exists
                // note: hash comparison not redundant, it's possible hash != h due to modulo
                return { bucket_pos + i, true };
            }
        }
        return { key_pos, false };
    }

    uint32_t N;
    uint32_t K;
    uint32_t NK;
    HashFunc hash_func;
    storage nodes;
    size_t num_nodes;
};
/*

TODO: Shift-forward hash map
Linear probed, but does a shift-forward upon collision
Keeps clusters of elements with the same target bucket contiguous and in order of target bucket

Shift-Forward Hash Map
     A linear probing hash map
     All elements with the same hash are contiguous in the internal array
     Such a sequence of elements is referred to as a "cluster"
     Lookup uses linear probe, but can terminate the probe once it finds a different hash (ie a different cluster) that is
     different to the lookup hash
     Insertion uses shift-forward to insert new elements;
     - The insert position is determined by probing to the end of the cluster, or the theoretical cluster position is used if
     one does not exist
     - if there is already an element in that position, it is moved to the end of its own cluster; "shifted forward"
     - repeat the shift-forward process until no elements are displaced.
     Deletion uses shift-back instead of tombstones.
     - If the shifted-back element was immediately followed by a cluster that is out of position,
         one element from that cluster is shifted back to the newly freed space. Repeat for subsequent clusters ad infinitum.
     This guarantees to eventually fill the hash map completely with no wasted space;
         time taken by operations should degrade fairly predictably.

void do_insert(const hash_map_node& new_node) {
    auto p = new_node.hash_mod_n;
    const auto sp = p;
    // find cluster for p
    while (nodes[p].alive()) {
        if (nodes[p].hash_mod_n <= sp) {
            // not found end of correct cluster yet, continue
            p = (p + 1) & (N - 1);
            assert(p != sp); // should never assert due to size check in insert()
        } else if (nodes[p].hash_mod_n > sp) {
            // overshot into next cluster
            // add the current node here. there is no cluster for this hash, create one
            auto old_node = nodes[p];
            nodes[p] = new_node;
            do_insert(old_node);
        }
    }
}

*/