#pragma once

#if !defined(__cplusplus) || __cplusplus < 201703L
#error C++17 is required
#endif

#include <cstdint>
#include <tuple>
#include <vector>

namespace cg {

template <typename T = int64_t>
class Graph {
  protected:
    const static size_t NONE = size_t(-1);
    size_t next_virtual_node;
    std::vector<size_t> edge;
    std::vector<T> edge_value;
    std::vector<size_t> edge_offset;
    std::vector<size_t> node_parent;
    std::vector<size_t> edge_parent;
    size_t num_nodes;

    void init(size_t n, const std::vector<std::tuple<size_t, size_t, T>>& edge_list) {
        next_virtual_node = NONE - 1;
        num_nodes = n;
        std::vector<size_t> offset(n + 1);
        for(const auto [a, b, c] : edge_list) {
            offset[a]++;
        }
        size_t pfsum = 0;
        for(size_t i = 0; i < offset.size(); i++) {
            size_t tmp = offset[i];
            offset[i] = pfsum;
            pfsum += tmp;
        }
        edge_offset = offset;
        edge.resize(offset.back());
        edge_value.resize(offset.back());
        edge_parent.resize(offset.back(), NONE);
        node_parent.resize(n, NONE);
        for(const auto [a, b, c] : edge_list) {
            edge[offset[a]] = b;
            edge_value[offset[a]] = c;
            offset[a]++;
        }
    }

  public:
    Graph(size_t n, const std::vector<std::tuple<size_t, size_t, T>>& edge_list) { init(n, edge_list); }
};

} // namespace cg
