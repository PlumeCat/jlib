// dag.h

#pragma once

#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>

template<typename Type, typename Storage>
class directed_acyclic_graph {
    // Storage vertices;
    std::unordered_map<std::string, Type> vertices;
    std::vector<std::pair<size_t, size_t>> edges;

public:
    directed_acyclic_graph(const directed_acyclic_graph&) = default;
    directed_acyclic_graph(directed_acyclic_graph&&) = default;
    directed_acyclic_graph& operator=(const directed_acyclic_graph&) = default;
    directed_acyclic_graph& operator=(directed_acyclic_graph&&) = default;

    void edge(size_t from, size_t to) {}
    size_t vertex(Type type) {
        vertices.emplace("hello");
    }
};

void foo() {

}