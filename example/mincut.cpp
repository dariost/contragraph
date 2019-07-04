#include "contragraph.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void print_graph(cg::Graph<int64_t>& g, const string& dir, size_t prog) {
    string epoch;
    if(prog < 100) {
        epoch += "0";
    }
    if(prog < 10) {
        epoch += "0";
    }
    epoch += to_string(prog);
    ofstream f(dir + "/" + epoch + ".dot");
    assert(f.good());
    f << g.dot(false) << endl;
    f.close();
}

int main(int argc, char* argv[]) {
    assert(argc == 2);
    string dir = argv[1];
    size_t n, m;
    cin >> n >> m;
    vector<tuple<size_t, size_t, int64_t>> edge;
    for(size_t i = 0; i < m; i++) {
        size_t a, b;
        int64_t c;
        cin >> a >> b >> c;
        edge.emplace_back(a, b, c);
        edge.emplace_back(b, a, c);
    }
    cg::Graph<int64_t> g(n, edge);
    size_t epoch = 0;
    size_t nn;
    size_t best_cut = size_t(-1);
    while((nn = g.get_num_nodes()) > 1) {
        print_graph(g, dir, epoch++);
        unordered_map<size_t, int64_t> value;
        unordered_map<size_t, bool> selected;
        vector<size_t> legal_order;
        legal_order.push_back(g.get_node(0));
        selected[g.get_node(0)] = true;
        while(legal_order.size() < nn) {
            size_t node = legal_order.back();
            for(size_t i = 0; i < g.get_num_edges(node); i++) {
                if(!selected[g.get_edge_neighbour(node, i)]) {
                    value[g.get_edge_neighbour(node, i)] += g.get_edge_value(node, i);
                }
            }
            size_t best = g.NONE, best_value = 0;
            for(auto [a, b] : value) {
                if(!selected[a] && b >= best_value) {
                    best_value = b;
                    best = a;
                }
            }
            assert(best != g.NONE);
            legal_order.push_back(best);
            selected[best] = true;
        }
        size_t last_node = legal_order.back();
        size_t cut_size = 0;
        for(size_t i = 0; i < g.get_num_edges(last_node); i++) {
            cut_size += g.get_edge_value(last_node, i);
        }
        if(cut_size < best_cut) {
            best_cut = cut_size;
            g.clear_marks();
            for(size_t i = 0; i < g.get_num_edges(last_node); i++) {
                g.mark_edge(last_node, i);
            }
        }
        legal_order.pop_back();
        g.contract(last_node, legal_order.back());
    }
    g.expand();
    print_graph(g, dir, epoch++);
    return 0;
}
