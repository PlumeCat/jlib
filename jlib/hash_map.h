// hash_map.h

#pragma once

#include <cstdint>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T, bool = std::is_integral_v<T>> struct default_hash_helper {
    static uint32_t hash(const T& t) {
        return (uint32_t)t;
    }
};

template<> struct default_hash_helper<std::string, false> {
    static uint32_t hash(const std::string& key) {
        const uint64_t p = 31; // 53
        const uint64_t m = 1e9 + 9;
        uint64_t hash = 0;
        uint64_t pp = 1;
        for (auto c : key) {
            hash = (hash + (c - 'a' + 1) * pp) % m;
            pp = (pp * p) % m;
        }
        return hash; // smaller than m so fits into u32
    }
};

template<typename Key> struct default_hash {
    uint32_t operator()(const Key& key) const {
        return default_hash_helper<Key>::hash(key);
    }
};

// simple open address hash map
// there are N slots for a given hash collision
// if they are exhausted, capacity is doubled (bucket size stays the same) and rehash everything
// TODO: replace with hopefully better performing "shift forward hash map"
template<typename Key, typename Value, typename HashFunc = default_hash<Key>, uint32_t EMPTY = std::numeric_limits<uint32_t>::max()>
class hash_map {
    template<typename OwnerType, typename PairType> struct iter {
    private:
        OwnerType* owner;
        uint32_t index;
        friend class hash_map;

    public:
        iter(OwnerType* owner, uint32_t index):
            owner(owner),
            index(index) {}
        PairType& operator*() {
            return owner->nodes[index].first;
        }
        PairType* operator->() {
            return &owner->nodes[index].first;
        }
        bool operator!=(const iter& i) const {
            return index != i.index;
        }
        bool operator==(const iter& i) const {
            return index == i.index;
        }

        iter& operator++() {
            while (index < owner->NK) {
                index++;
                if (index >= owner->NK) {
                    break;
                }
                if (owner->nodes[index].second != EMPTY) {
                    break;
                }
            }
            return *this;
        }
        iter operator++(int) {
            auto t = *this;
            ++(*this);
            return t;
        }
    };

public:
    // key, value, hash (before mod)
    using type = hash_map<Key, Value, HashFunc, EMPTY>;
    using kv_pair = std::pair<Key, Value>;
    using node = std::pair<kv_pair, uint32_t>;
    using iterator = iter<hash_map, kv_pair>;
    using const_iterator = iter<const hash_map, const kv_pair>;

    hash_map(const hash_map&) = default;
    hash_map(hash_map&&) = default;
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&&) = default;
    explicit hash_map(uint32_t num_clusters = 16, uint32_t cluster_size = 2):
        N(num_clusters),
        K(cluster_size),
        NK(num_clusters * cluster_size),
        nodes(
            num_clusters * cluster_size,
            {
                {Key(), Value()},
                EMPTY
    }
        ),
        first_node(EMPTY), last_node(EMPTY), num_nodes(0) {}

    hash_map(const std::initializer_list<kv_pair>& initializer):
        hash_map() {
        for (auto& [k, v] : initializer) {
            insert(k, v);
        }
    }

    // Iterators
    const_iterator begin() const {
        return { this, num_nodes ? first_node : NK };
    }
    const_iterator end() const {
        return { this, NK };
    }
    iterator begin() {
        return { this, num_nodes ? first_node : NK };
    }
    iterator end() {
        return { this, NK };
    }

    // Size
    uint32_t size() const {
        return num_nodes;
    }
    bool empty() const {
        return size() == 0;
    }

    // Clear
    void clear() {
        nodes.assign(
            nodes.size(),
            {
                {Key(), Value()},
                EMPTY
        }
        );
        first_node = EMPTY;
        last_node = EMPTY;
        num_nodes = 0;
    }

    // Find/contains
    bool contains(const Key& key) const {
        return find(key) != end();
    }
    iterator find(const Key& key) {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        for (auto i = 0u; i < K; i++) {
            if (nodes[pos + i].second == h && nodes[pos + i].first.first == key) {
                return { this, pos + i };
            }
        }
        return end();
    }
    const_iterator find(const Key& key) const {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        for (auto i = 0u; i < K; i++) {
            if (nodes[pos + i].second == h && nodes[pos + i].first.first == key) {
                return { this, pos + i };
            }
        }
        return end();
    }

    const Value& operator[](const Key& key) const {
        return get(key);
    }
    Value& operator[](const Key& key) {
        auto index = uint32_t { 0 };
        if (find_or_suggest(key, index)) {
            return nodes[index].first.second;
        } else {
            return nodes[insert(key, Value()).index].first.second;
        }
    }

    Value& get(const Key& key) {
        auto iter = find(key);
        if (iter == end()) {
            NOT_FOUND(key);
        }
        return iter->second;
    }
    const Value& get(const Key& key) const {
        auto iter = find(key);
        if (iter == end()) {
            NOT_FOUND(key);
        }
        return iter->second;
    }
    bool get(const Key& key, Value& out_value) {
        auto iter = find(key);
        if (iter == end()) {
            return false;
        }
        out_value = iter->second;
        return true;
    }
    // get value or return default value
    // will not insert default value into hash map
    Value get_or(const Key& key, const Value& default_value) {
        auto iter = find(key);
        if (iter == end()) {
            return default_value;
        }
        return iter->second;
    }

    iterator insert(const Key& key, const Value& value) {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        auto candidate = EMPTY;

        for (auto i = 0u; i < K; i++) {
            if (candidate == EMPTY && nodes[pos + i].second == EMPTY) {
                candidate = pos + i;
            } else if (nodes[pos + i].second == h && nodes[pos + i].first.first == key) {
                ALREADY_EXISTS(key);
            }
        }

        if (candidate == EMPTY) {
            // ran out of slots, rehash needed
            resize();
            return insert(key, value);
        } else {
            nodes[candidate] = {
                {key, value},
                h
            };
            first_node = (first_node == EMPTY) ? candidate : std::min(first_node, candidate);
            last_node = (last_node == EMPTY) ? candidate : std::max(last_node, candidate);
            num_nodes += 1;
            return { this, candidate };
        }
    }

    void update(const Key& key, const Value& value) {
        auto iter = find(key);
        if (iter == end()) {
            NOT_FOUND(key);
        }
        iter->second = value;
    }

    void remove(const Key& key) {
        auto iter = find(key);
        if (iter == end()) {
            NOT_FOUND(key);
        }
        nodes[iter.index] = {
            {Key(), Value()},
            EMPTY
        };
        // *(iter.ptr) = { { Key(), Value() }, EMPTY };
    }

private:
    // TODO: remove
    void ALREADY_EXISTS([[maybe_unused]] const Key& key) const {
        throw std::runtime_error("Already exists");
    }
    void NOT_FOUND([[maybe_unused]] const Key& key) const {
        throw std::runtime_error("Not found");
    }
    void resize() {
        // double N, and rehash everything
        // TODO: pretty inefficient
        auto new_hash_map = type(N * 2, K);
        for (iterator i = begin(); i != end(); i++) {
            new_hash_map.insert(i->first, i->second);
        }
        *this = std::move(new_hash_map);
    }
    // if the element exists, return true and out_index contains the internal index
    // if the element does not exist, return false, and out_index contains the position to insert
    // if false and out_index is EMPTY, insert will cause resize
    bool find_or_suggest(const Key& key, uint32_t& out_index) const {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        auto candidate = EMPTY;
        for (auto i = 0u; i < K; i++) {
            if (nodes[pos + i].second == EMPTY) {
                candidate = (candidate == EMPTY) ? pos + i : candidate;
            } else if (nodes[pos + i].second == h && nodes[pos + i].first.first == key) {
                // already exists
                out_index = pos + i;
                return true;
            }
        }
        out_index = candidate;
        return false;
    }

    uint32_t N;
    uint32_t K;
    uint32_t NK;
    HashFunc hash_func;
    std::vector<node> nodes;
    uint32_t first_node;
    uint32_t last_node;
    uint32_t num_nodes;
};

// /*

// TODO: Shift-forward hash map
// Linear probed, but does a shift-forward upon collision
// Keeps clusters of elements with the same target bucket contiguous and in order of target bucket

// Shift-Forward Hash Map
//      A linear probing hash map
//      All elements with the same hash are contiguous in the internal array
//      Such a sequence of elements is referred to as a "cluster"
//      Lookup uses linear probe, but can terminate the probe once it finds a different hash (ie a different cluster) that is
//      higher than the lookup hash Insertion uses shift-forward to insert new elements;
//      - The insert position is determined by probing to the end of the cluster, or the theoretical cluster position is used if
//      one does not exist
//      - if there is already an element in that position, it is moved to the end of its own cluster; "shifted forward"
//      - repeat the shift-forward process until no elements are displaced.
//      Deletion uses shift-back instead of tombstones.
//      - If the shifted-back element was immediately followed by a cluster that is out of position,
//          one element from that cluster is shifted back to the newly freed space. Repeat for subsequent clusters ad infinitum.
//      This guarantees to eventually fill the hash map completely with no wasted space;
//          time taken by operations should degrade fairly predictably.

// void do_insert(const hash_map_node& new_node) {
//     auto p = new_node.hash_mod_n;
//     const auto sp = p;
//     // find cluster for p
//     while (nodes[p].alive()) {
//         if (nodes[p].hash_mod_n <= sp) {
//             // not found end of correct cluster yet, continue
//             p = (p + 1) & (N - 1);
//             assert(p != sp); // should never assert due to size check in insert()
//         } else if (nodes[p].hash_mod_n > sp) {
//             // overshot into next cluster
//             // add the current node here. there is no cluster for this hash, create one
//             auto old_node = nodes[p];
//             nodes[p] = new_node;
//             do_insert(old_node);
//         }
//     }
// }