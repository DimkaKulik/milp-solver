#include <bits/stdc++.h>
#include "utility.h"
#include "direct_method.h"
#include "dual_method.h"


int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Pass filenames via command line arguments" <<
                     "(example: ./executable ../edges.txt ../nodes.txt)" << std::endl;
        return 0;
    }
    std::string edges_filename = argv[1];
    std::string nodes_filename = argv[2];

    std::vector<Edge> edges;
    std::vector<Node> nodes;
    std::vector<std::vector<int64_t>> graph;
    ReadGraph(edges_filename, nodes_filename, &edges, &nodes, &graph);
    
    auto [flow, basis_edges] = std::move(GetInitialFlow(edges, nodes, graph));

    auto dual_method_flow = std::move(DualMethod(edges, nodes, graph, basis_edges));
    for (int64_t i = 0; i < int64_t{edges.size()}; ++i) {
        std::cerr << "edge: " << edges[i].from << " " << edges[i].to << " " << dual_method_flow[i] << std::endl;
    }

    // auto flow = std::move(Solve(edges, nodes, graph));


    // std::set<int64_t> basis_edges;
    // basis_edges.insert(0);
    // basis_edges.insert(1);
    // basis_edges.insert(2);
    // basis_edges.insert(3);
    // basis_edges.insert(4);
    // std::vector<int64_t> flow(8);
    // flow[0] = 20;
    // flow[1] = 40;
    // flow[2] = 0;
    // flow[3] = 5;
    // flow[4] = 25;
    // flow[5] = 55;
    // flow[6] = 50;
    // flow[7] = 40;


    // Method(edges, nodes, graph, flow, basis_edges); 
    
    // for (auto elem : flow) {
    //     std::cerr << elem << " " << std::endl;
    // }
}