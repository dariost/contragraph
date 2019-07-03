#include <iostream>

#include "contragraph.hpp"

using namespace std;

int main() {
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
    g.contract(1, 2);
    cout << g.dot(false) << endl;
    return 0;
}
