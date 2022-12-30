// hash_map.h

#pragma once

#include <vector>
#include <string>
#include <exception>
#include "log.h"


// class KeyError: std::exception {};

// template<typename Key>
// struct DefaultHash {
//     static uint64_t hash(const Key&);
// };

// template<>
// struct DefaultHash<std::string> {
//     static uint64_t hash(const std::string& key) {
//         const uint64_t p = 31; // 53
//         const uint64_t m = 1e9 + 9;
//         uint64_t hash = 0;
//         uint64_t pp = 1;
//         for (auto c: key) {
//             hash = (hash + (c - 'a' + 1) * pp) % m;
//             pp = (pp * p) % m;
//         }
//         return hash;
//     }
// };


/*
    begin
    end

    size / empty
    clear

    find_cluster(k):
        // returns start of cluster for k
        return hash(k) % N

    find_index(k)
        // returns the open index of k
        // ie the actual physical location of k in the internal array
        p = find_cluster(k)
        while nodes[p].alive
            if nodes[p].k == k
                return p // maybe return a writable iterator?
            p = p + 1 // mod N
        ERROR_NOT_FOUND
    
    get(k)
        // get value at k, or ERROR_NOT_FOUND
        index = find(k)
        return values[index]

    update(k, v)
        // update value at k, or ERROR_NOT_FOUND
        index = find(k)
        nodes[index] = k
        values[index] = k

    insert(k, v)
        h = hash(k)
        p = find_cluster(k) // start of cluster for k
        
        while nodes[p].alive
            p = p + 1 // mod N
            
            if nodes[p].hash != h
                // ran into next cluster
                // insert the current node here, then continue the insert process with the first node from the next cluster
                // if this runs into the next-next cluster, repeat with first node of next-next cluster etc ad infinitum
                // if we run out of space, exception
                old_node = nodes[p]
                old_value = values[p]

                nodes[p] = { k, h }
                values[p] = v

                k = old_node.key
                h = old_node.hash
                v = old_value

        nodes[p] = { k, h }
        values[p] = v

    delete(k)
        h = hash(k)
        p = find_cluster(k)

        if nodes[p].key == k
            nodes[p].alive = false


        shift_back_pos = p
        while true
            if nodes[p+1].alive
                p = p + 1 // mod N
                if nodes[p].hash != h
                    nodes[shift_back_pos] = nodes[p]
                    values[shift_back_pos] = values[p]
                    
                    nodes[p].alive = false
                    shift_back_pos = p




*/


/*

Shift-Forward Hash Map

Definitions:
    N: the size of the internal array of the hashmap
    Element: a (key, value) tuple
    Hash: the hash function H : K |-> u64
    Slot: one of the slots in the internal array.
            Theoretical Slot:
                an element's "theoretical slot" is determined by hash(key) % N
                multiple elements may have the same theoretical slot, but of course
                a slot can only contain one element...
            Actual Slot:
                if an element is inserted but the theoretical slot was occupied,

    Cluster: sequence of elements with the same hash, see below
    Probe: linear search through elements (generally fowards and wrapping to 0 at the end of the array)

Invariants:
    
    All elements with the same hash are contiguous in the internal array
    Such a sequence of elements is referred to as a "cluster"

    Lookup works as with normal hash map 

    A*B*C*
    A A A C C C
    A A A B C C C




*/



// template<typename Key, typename Value, typename Hash = DefaultHash<Key>>
static const uint64_t EMPTY = std::numeric_limits<uint64_t>::max();
struct hash_map {
    using Key = std::string;
    using Value = std::string;
    
    static uint64_t Hash(const std::string& k) {
        return k.size(); // DELIBERATELY BAD HASH FOR TESTING
    }

    std::vector<uint64_t> hashes;
    std::vector<Key> keys;
    std::vector<Value> values;
    uint64_t N;

public:
    hash_map(const hash_map&) = default;
    hash_map(hash_map&&) = default;
    hash_map& operator=(const hash_map&) = default;
    hash_map& operator=(hash_map&&) = default;

    hash_map(uint64_t n = 1024): N(n) {
        hashes.resize(N, EMPTY);
        keys.resize(N); // all dead at first
        values.resize(N);
    }

    Value& get(const Key& key) {
        auto p = get_index(key);
        return values[p];
    }
    void set(const Key& key, const Value& value) {
        auto p = get_index(key);
        values[p] = value;
    }
    
    
    void insert(const Key& key, const Value& value) {
        auto h = Hash(key);
        auto p = find_cluster(key);
        if (hashes[p] == h) {
            //
        }        
    }

    void erase(const Key& key) {
        auto k = key;
        auto h = Hash(key);
        auto p = find_cluster(key);
    }

    template<typename T>
    void foreach(T t) {
        for (auto i = 0; i < N; i++) {
            // if (hashes[i] != EMPTY) {
                t(keys[i], values[i]);
            // }
        }
    }

private:
    // find the START of the cluster for this key
    // note that cluster is not guaranteed to start at hash(key) % N because the previous cluster might be very long;
    // we have to probe forward until find a matching hash
    uint64_t find_cluster(const Key& key) {
        auto h = Hash(key);
        auto p = h & (N - 1);
        auto start_p = p;
        while (hashes[p] != EMPTY && hashes[p] != h) {
            p = (p + 1) & (N - 1);
            if (p == start_p) {
                throw std::runtime_error("ERROR_OVERFLOWING");
            }
        }
        return p;
    }
    uint64_t get_index(const Key& key) {
        auto c = find_cluster(key);
        while (hashes[c] != EMPTY) {
            if (keys[c] == key) {
                return c;
            }
            c = (c + 1) & (N - 1);
        }
        throw std::runtime_error("ERROR_NOT_FOUND");
    }
};

// TODO: find_cluster() is O(N)
// It might be possible to cache cluster starts if the bookkeeping does not become too arduous
// This will make find_cluster() more like O(1)
// (because clusters shift forwards and backwards in fairly predictable ways)
