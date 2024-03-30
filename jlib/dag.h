// dag.h

#pragma once

#include <vector>
#include <tuple>

template<typename Type, typename Storage>
class directed_acyclic_graph {
    // Storage vertices;
    std::vector<Type> vertices;
    std::vector<std::pair<size_t, size_t>> edges;

public:
    directed_acyclic_graph(const directed_acyclic_graph&) = default;
    directed_acyclic_graph(directed_acyclic_graph&&) = default;
    directed_acyclic_graph& operator=(const directed_acyclic_graph&) = default;
    directed_acyclic_graph& operator=(directed_acyclic_graph&&) = default;

    void edge(size_t from, size_t to) {
        edges.emplace_back(from, to);
    }
    template<typename... Args>
    void vertex(Args&&... args) {
        vertices.emplace_back(std::forward(args...));
    }
};

// TODO: topo sort