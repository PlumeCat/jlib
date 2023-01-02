// hash_map.h

#pragma once

#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <utility>


class KeyError: std::exception {};

static const uint32_t EMPTY = std::numeric_limits<uint32_t>::max();

template<typename Key>
struct default_hash {
    uint32_t operator()(const Key&) const;
};

template<>
struct default_hash<std::string> {
    uint32_t operator()(const std::string& key) const {
        const uint64_t p = 31; // 53
        const uint64_t m = 1e9 + 9;
        uint64_t hash = 0;
        uint64_t pp = 1;
        for (auto c: key) {
            hash = (hash + (c - 'a' + 1) * pp) % m;
            pp = (pp * p) % m;
        }
        return hash; // smaller than m so fits into u32
    }
};

// simple open address hash map
// there are N slots for a given hash collision
// if they are exhausted, capacity is doubled (bucket size stays the same) and rehash everything
// TODO: replace with hopefully better performing "shift forward hash map"
template<typename Key, typename Value, typename HashFunc = default_hash<Key>>
struct hash_map {
    template<typename OwnerType, typename PairType> struct iter {
    private:
        OwnerType* owner;
        uint32_t index;

    public:
        iter(OwnerType* owner, uint32_t index): owner(owner), index(index) {}
        PairType& operator*() { return owner->nodes[index].first; }
        PairType* operator->() { return &owner->nodes[index].first; }
        bool operator != (const iter& i) { return index != i.index; }
        iter& operator++()   {
            while (index < owner->NK && owner->nodes[++index].second == EMPTY) {};
            return *this;
        }
        iter operator++(int) {
            auto t = *this;
            while (index < owner->NK && owner->nodes[++index].second == EMPTY) {};
            return t;
        }
    };
    
    // key, value, hash (before mod)
    using kv_pair = std::pair<Key, Value>;
    using node = std::pair<kv_pair, uint32_t>;
    using iterator = iter<hash_map, kv_pair>;
    using const_iterator = iter<const hash_map, const kv_pair>;

    uint32_t N;
    uint32_t K;
    uint32_t NK;
    HashFunc hash_func;
    std::vector<node> nodes;
    uint32_t first_node;
    uint32_t last_node;
    uint32_t num_nodes;

    hash_map(const hash_map&) = default;
    hash_map(hash_map&&) = default;
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&&) = default;
    hash_map(uint32_t num_clusters = 16, uint32_t cluster_size = 2):
        nodes(num_clusters * cluster_size, { { Key(), Value() }, EMPTY }),
        N(num_clusters),
        K(cluster_size),
        NK(num_clusters * cluster_size),
        first_node(EMPTY),
        last_node(EMPTY),
        num_nodes(0) {}
    
    // Iterators
    iterator begin() { return { this, num_nodes ? first_node : NK }; }
    iterator end() { return { this, NK }; }
    const_iterator begin() const { return { this, num_nodes ? first_node : NK }; }
    const_iterator end() const { return { this, NK }; }
    
    // Size
    uint32_t size() const { return num_nodes; }
    bool empty() const { return size() == 0; }
    
    // Clear
    void clear() {
        nodes.assign(nodes.size(), { { Key(), Value() }, EMPTY });
        first_node = EMPTY;
        last_node = EMPTY;
        num_nodes = 0;
    }

    // Find/contains
    bool contains(const Key& key) const { return find(key) != end(); }
    iterator find(const Key& key) {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        for (auto i = 0; i < K; i++) {
            if (nodes[pos + i].second != EMPTY && nodes[pos + i].first.first == key) {
                return { this, pos + i };
            }
        }
        return end();
    }
    const_iterator find(const Key& key) const {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        for (auto i = 0; i < K; i++) {
            if (nodes[pos + i].second != EMPTY && nodes[pos + i].first.first == key) {
                return { this, pos + i };
            }
        }
        return end();
    }

    // Create, read, update, delete
    Value& operator[](const Key& key) { return get(key); }
    Value& get(const Key& key) {
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
    Value& get_or(const Key& key, const Value& default_value) {
        auto iter = find(key);
        if (iter == end()) {
            return default_value;
        }
        return iter->second;
    }

    void insert(const Key& key, const Value& value) {
        auto h = hash_func(key);
        auto bucket = h & (N - 1);
        auto pos = bucket * K;
        for (auto i = 0; i < K; i++) {
            if (nodes[pos + i].second == EMPTY) {
                first_node = (first_node == EMPTY) ? pos + i : std::min(first_node, pos + i);
                last_node = (last_node == EMPTY) ? pos + i : std::max(last_node, pos + i);
                num_nodes += 1;
                nodes[pos + i] = { { key, value }, h };
                return;
            }
        }

        // ran out of slots, rehash needed
        resize();
        insert(key, value);
    }

    Value update(const Key& key, const Value& value) {
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
        *(iter.ptr) = { { Key(), Value() }, EMPTY };
    }


private:
    // TODO: remove
    void NOT_FOUND(const Key& key) const {
        throw std::runtime_error("Not found");
    }
    void resize() {
        // double N, and rehash everything
        // TODO: pretty inefficient
        auto new_hash_map = hash_map<Key, Value, HashFunc>(N * 2, K);
        for (auto& i: *this) {
            new_hash_map.insert(i.first, i.second);
        }
        // printf("REHASHING\n");
        *this = std::move(new_hash_map);
    }
};


// /*


// TODO: Shift-forward hash map
// Linear probed, but does a shift-forward upon collision
// Keeps clusters of elements with the same target bucket contiguous and in order of target bucket

/*

    struct Node { alive, key, hash, cluster, value }
    struct Iterator {}

    N = 2 ** M for some integer M
    Node nodes[N]
    
    
    begin() / end() -> Iterator
    size() / empty() / clear()

    find_index(k)
        // returns the open index of k
        // ie the actual physical location of k in the internal array
        p = hash(k) % N
        while nodes[p].alive
            if nodes[p].key == k
                return p // maybe return a writable iterator?
            p = p + 1 // mod N
        ERROR_NOT_FOUND
    
    get(k)
        // get value at k, or ERROR_NOT_FOUND
        index = find_index(k)
        return nodes[index].value

    update(k, v)
        // update value at k, or ERROR_NOT_FOUND
        index = find_index(k)
        nodes[index] = { k, v }

    insert(k, v)
        // find cluster for k

        h = hash(k)
        p = h % N
        start_p = p

        while nodes[p].alive
            // probe forward to find our cluster

            if nodes[p].cluster < p
                //
            elif nodes[p].cluster == p
                // in correct cluster
            elif nodes[p].cluster > p
                // overshoot



    delete(k)

*/

// */


// /*

// Shift-Forward Hash Map

// Definitions:
//     N: the size of the internal array of the hashmap
//     Element: a (key, value) tuple
//     Hash: the hash function H : K |-> u64
//     Slot: one of the slots in the internal array.
//     Cluster: sequence of elements with the same hash
//     Probe: linear search through elements (generally fowards and wrapping to 0 at the end of the array)

// Invariants:
    
//     All elements with the same hash are contiguous in the internal array
//     Such a sequence of elements is referred to as a "cluster"

//     Lookup works as with normal hash map 

//     A*B*C*
//     A A A C C C
//     A A A B C C C




// */



// // template<typename Key, typename Value, typename Hash = default_hash<Key>>
// static const uint64_t EMPTY = std::numeric_limits<uint64_t>::max();
// using Key = std::string;
// using Value = std::string;
// static const auto ERROR_OVERFLOW = runtime_error("Error: Insert would cause overflow");
// static const auto ERROR_NOTFOUND = runtime_error("Error: Not Found");

// struct hash_map_node {
//     uint64_t hash;
//     uint64_t hash_mod_n; // hash % N
//     Key key;
//     Value value;

//     inline bool alive() const { return cluster != EMPTY; }
// };
// static const auto EMPTY_NODE = hash_map_node { 0, EMPTY };

// struct hash_map {
    
//     static uint64_t Hash(const std::string& k) {
//         return k.size(); // DELIBERATELY BAD HASH FOR TESTING
//     }


//     // std::vector<uint64_t> hashes;
//     // std::vector<Key> keys;
//     // std::vector<Value> values;
//     vector<hash_map_node> nodes;
//     uint64_t alive_count = 0;
//     uint64_t N;

// public:
//     hash_map(const hash_map&) = default;
//     hash_map(hash_map&&) = default;
//     hash_map& operator=(const hash_map&) = default;
//     hash_map& operator=(hash_map&&) = default;

//     hash_map(uint64_t n = 1024): N(n) {
//         nodes.resize(N, EMPTY_NODE);
//     }

//     Value& get(const Key& key) {
//         auto p = get_index(key);
//         return nodes[p].value;
//     }
//     void set(const Key& key, const Value& value) {
//         auto p = get_index(key);
//         nodes[p].value = value;
//     }
    
    
//     void insert(const Key& key, const Value& value) {
//         auto h = Hash(key);
//         auto p = h & (N - 1);
//         // TODO: make sure element does not exist

//         if (alive_count == N - 1) {
//             throw ERROR_OVERFLOW;
//         }

//         do_insert({ h, p, key, value });

//         alive_count += 1;
//     }


// private:
//     void do_insert(const hash_map_node& new_node) {
//         auto p = new_node.hash_mod_n;
//         const auto sp = p;

//         // find cluster for p
//         while (nodes[p].alive()) {
//             if (nodes[p].hash_mod_n <= sp) {
//                 // not found end of correct cluster yet, continue
//                 p = (p + 1) & (N - 1);
//                 assert(p != sp); // should never assert due to size check in insert()
//             } else if (nodes[p].hash_mod_n > sp) {
//                 // overshot into next cluster
//                 // add the current node here. there is no cluster for this hash, create one
//                 auto old_node = nodes[p];
//                 nodes[p] = new_node;
//                 do_insert(old_node);
//             }
//         }
//     }
// public:



//     void erase(const Key& key) {
//     }

//     template<typename T>
//     void foreach(T t) {
//         for (auto i = 0; i < N; i++) {
//             // if (hashes[i] != EMPTY) {
//                 t(nodes[i].key, nodes[i].value);
//             // }
//         }
//     }

// private:
//     // find the START of the cluster for this key
//     // note that cluster is not guaranteed to start at hash(key) % N because the previous cluster might be very long;
//     // we have to probe forward until find a matching hash
//     uint64_t find_cluster(const Key& key, uint64_t& out_hash) {
//         auto h = Hash(key);
//         out_hash = h;
//         auto p = h & (N - 1);
//         auto start_p = p;
//         while (nodes[p].alive() && nodes[p].hash != h) {
//             p = (p + 1) & (N - 1);
//             if (p == start_p) {
//                 throw std::runtime_error("ERROR_OVERFLOWING");
//             }
//         }
//         return p;
//     }
//     uint64_t get_index(const Key& key) {
//         auto h = uint64_t{0};
//         auto c = find_cluster(key, h);
//         while (nodes[c].alive()) {
//             if (nodes[c].hash == h && nodes[c].key == key) {
//                 return c;
//             }
//             c = (c + 1) & (N - 1);
//         }
//         throw std::runtime_error("ERROR_NOT_FOUND");
//     }
// };

// // TODO: find_cluster() is O(N)
// // It might be possible to cache cluster starts if the bookkeeping does not become too arduous
// // This will make find_cluster() more like O(1)
// // (because clusters shift forwards and backwards in fairly predictable ways)
