#pragma once

#if !defined(__cplusplus) || __cplusplus < 201703L
#error C++17 is required
#endif

#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <numeric>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace cg {

template <typename T = int64_t>
class Graph {
  protected:
    const size_t NONE = size_t(-1);
    std::vector<size_t> edge;
    std::vector<T> edge_value;
    std::vector<size_t> edge_offset;
    std::vector<size_t> node_parent;
    std::vector<size_t> edge_parent;
    std::vector<size_t> current_node_pointer;
    std::vector<size_t> current_nodes;
    std::vector<bool> edge_marked;
    std::vector<size_t> edge_transposed;
    std::vector<size_t> edge_offset_transposed;
    size_t num_nodes;
    std::function<T(const T&, const T&)> fold_function;
    bool self_loops;

    void init(size_t n, const std::vector<std::tuple<size_t, size_t, T>>& edge_list, std::function<T(const T&, const T&)> fold_fun,
              bool no_self_loops) {
        self_loops = !no_self_loops;
        fold_function = fold_fun;
        num_nodes = n;
        std::vector<size_t> offset(n + 1);
        std::vector<size_t> offset_transposed(n + 1);
        for(const auto [a, b, c] : edge_list) {
            offset[a]++;
            offset_transposed[b]++;
        }
        size_t pfsum = 0;
        size_t pfsum_transposed = 0;
        for(size_t i = 0; i < n + 1; i++) {
            size_t tmp = offset[i];
            offset[i] = pfsum;
            pfsum += tmp;
            size_t tmp_transposed = offset_transposed[i];
            offset_transposed[i] = pfsum_transposed;
            pfsum_transposed += tmp_transposed;
        }
        edge_offset = offset;
        edge_offset_transposed = offset_transposed;
        edge.resize(offset.back());
        edge_transposed.resize(offset_transposed.back());
        edge_value.resize(offset.back());
        edge_parent.resize(offset.back(), NONE);
        node_parent.resize(n, NONE);
        current_nodes.resize(n);
        current_node_pointer.resize(n);
        edge_marked.resize(offset.back());
        for(const auto [a, b, c] : edge_list) {
            edge[offset[a]] = b;
            edge_value[offset[a]] = c;
            offset[a]++;
            edge_transposed[offset_transposed[b]] = a;
            offset_transposed[b]++;
        }
        std::iota(current_nodes.begin(), current_nodes.end(), 0UL);
        std::iota(current_node_pointer.begin(), current_node_pointer.end(), 0UL);
    }

    size_t node_find(size_t node) {
        if(node_parent[node] == NONE) {
            return node;
        }
        return node_parent[node] = node_find(node_parent[node]);
    }

    size_t edge_find(size_t edge) {
        if(edge_parent[edge] == NONE) {
            return edge;
        }
        return edge_parent[edge] = edge_find(edge_parent[edge]);
    }

  public:
    Graph(
        size_t n, const std::vector<std::tuple<size_t, size_t, T>>& edge_list,
        std::function<T(const T&, const T&)> fold_fun = [](const T& a, const T& b) { return a + b; }, bool no_self_loops = true) {
        init(n, edge_list, fold_fun, no_self_loops);
    }

    size_t get_num_nodes() { return current_nodes.size(); }

    size_t get_node(size_t index) { return current_nodes[index]; }

    size_t get_num_edges(size_t node) { return edge_offset[node + 1] - edge_offset[node]; }

    const T& get_edge_value(size_t node, size_t index) { return edge_value[edge_offset[node] + index]; }

    size_t get_edge_neighbour(size_t node, size_t index) { return node_find(edge[edge_offset[node] + index]); }

    std::tuple<size_t, const T&> get_edge(size_t node, size_t index) { return {get_edge_neighbour(), get_edge_value()}; }

    void mark_edge(size_t node, size_t index) { edge_marked[edge_offset[node] + index] = true; }

    size_t contract(size_t node_a, size_t node_b) {
        assert(node_a != node_b);
        size_t new_index = node_parent.size();
        node_parent.push_back(NONE);
        node_parent[node_find(node_a)] = node_parent[node_find(node_b)] = new_index;
        std::unordered_map<size_t, size_t> mapper;
        std::unordered_map<size_t, size_t> incident;
        edge_offset.push_back(edge_offset.back());
        for(size_t x : {node_a, node_b}) {
            for(size_t i = edge_offset[x]; i < edge_offset[x + 1]; i++) {
                size_t true_dest = node_find(edge[i]);
                if(!self_loops && true_dest == node_find(x)) {
                    continue;
                }
                if(mapper.count(true_dest)) {
                    size_t true_i = mapper[true_dest];
                    edge_parent[edge_find(i)] = true_i;
                    edge_value[true_i] = fold_function(edge_value[true_i], edge_value[i]);
                } else {
                    edge_parent[edge_find(i)] = edge.size();
                    mapper[true_dest] = edge.size();
                    edge_parent.push_back(NONE);
                    edge.push_back(true_dest);
                    edge_offset.back()++;
                    edge_marked.push_back(false);
                    edge_value.push_back(edge_value[i]);
                }
            }
            for(size_t i = edge_offset_transposed[x]; i < edge_offset_transposed[x + 1]; i++) {
                size_t e = node_find(edge_transposed[i]);
                if(e != new_index) {
                    incident[e]++;
                }
            }
        }
        current_node_pointer.push_back(current_nodes.size());
        current_nodes.push_back(new_index);
        mapper.clear();
        edge_offset_transposed.push_back(edge_offset_transposed.back());
        for(auto [e, v] : incident) {
            edge_transposed.push_back(e);
            edge_offset_transposed.back()++;
        }
        for(auto [e, v] : incident) {
            if(v < 2) {
                continue;
            }
            size_t new_node = node_parent.size();
            node_parent.push_back(NONE);
            node_parent[node_find(e)] = new_node;
            edge_offset.push_back(edge_offset.back());
            std::unordered_set<size_t> incident_tmp;
            for(size_t i = edge_offset[e]; i < edge_offset[e + 1]; i++) {
                size_t true_dest = node_find(edge[i]);
                if(mapper.count(true_dest)) {
                    size_t true_i = mapper[true_dest];
                    edge_parent[edge_find(i)] = true_i;
                    edge_value[true_i] = fold_function(edge_value[true_i], edge_value[i]);
                } else {
                    edge_parent[edge_find(i)] = edge.size();
                    mapper[true_dest] = edge.size();
                    edge_parent.push_back(NONE);
                    edge.push_back(true_dest);
                    edge_offset.back()++;
                    edge_marked.push_back(false);
                    edge_value.push_back(edge_value[i]);
                }
            }
            for(size_t i = edge_offset_transposed[e]; i < edge_offset_transposed[e + 1]; i++) {
                incident_tmp.insert(node_find(edge_transposed[i]));
            }
            edge_offset_transposed.push_back(edge_offset_transposed.back());
            for(size_t m : incident_tmp) {
                edge_transposed.push_back(m);
                edge_offset_transposed.back()++;
            }
            current_node_pointer.push_back(current_node_pointer[e]);
            current_node_pointer[e] = NONE;
            current_nodes[current_node_pointer[new_node]] = new_node;
        }
        for(size_t x : {node_a, node_b}) {
            if(current_nodes.back() == x) {
                current_nodes.pop_back();
                current_node_pointer[x] = NONE;
            } else {
                size_t to_preserve = current_nodes.back();
                std::swap(current_nodes[current_node_pointer[x]], current_nodes.back());
                current_nodes.pop_back();
                current_node_pointer[to_preserve] = current_node_pointer[x];
                current_node_pointer[x] = NONE;
            }
        }
        return new_index;
    }

    std::string dot(bool directed = true) {
        std::stringstream s;
        if(directed) {
            s << "digraph g { ";
        } else {
            s << "graph g { ";
        }
        std::unordered_set<size_t> visited;
        std::vector<std::tuple<size_t, size_t, int64_t>> edges;
        std::map<std::tuple<size_t, size_t>, size_t> edge_mapper;
        std::function<void(size_t)> dfs = [&dfs, &visited, &s, &edges, &edge_mapper, &directed, this](size_t node) {
            if(visited.count(node)) {
                return;
            }
            visited.insert(node);
            for(size_t i = 0; i < this->get_num_edges(node); i++) {
                size_t e = this->get_edge_neighbour(node, i);
                const T& v = this->get_edge_value(node, i);
                if(directed) {
                    edges.push_back({node, e, v});
                } else {
                    size_t a = std::min(node, e);
                    size_t b = std::max(node, e);
                    if(!edge_mapper.count({a, b})) {
                        edge_mapper[{a, b}] = edges.size();
                        edges.push_back({a, b, v});
                    }
                }
                dfs(e);
            }
        };
        size_t n = this->get_num_nodes();
        for(size_t i = 0; i < n; i++) {
            dfs(this->get_node(i));
        }
        for(const auto [a, b, c] : edges) {
            if(directed) {
                s << a << " -> " << b << " [label=" << c << "]; ";
            } else {
                s << a << " -- " << b << " [label=" << c << "]; ";
            }
        }
        s << "}";
        return s.str();
    }
};

} // namespace cg
