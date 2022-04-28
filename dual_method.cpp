#include "dual_method.h"


std::vector<int64_t> GetPotentialsDualMethod(const std::vector<Edge>& edges, 
                                             const std::vector<Node>& nodes,
                                             const std::set<int64_t>& basis_edges) {
    if (nodes.empty()) { 
        std::cerr << "dual_method.cpp/8/Empty nodes" << std::endl;
        throw "Empty nodes.\n";
    }

    std::vector<bool> calculated(nodes.size(), false);
    std::vector<int64_t> potentials(nodes.size());

    calculated[0] = true;
    potentials[0] = 0;

    for (int64_t i = 1; i < int64_t{nodes.size()}; ++i) {
        for (auto basis_edge_index : basis_edges) {
            int64_t u = edges[basis_edge_index].from;
            int64_t v = edges[basis_edge_index].to;
            int64_t cost = edges[basis_edge_index].cost;

            if (calculated[u] && !calculated[v]) {
                potentials[v] = potentials[u] + cost;
                calculated[v] = true;
            }

            if (!calculated[u] && calculated[v]) {
                potentials[u] = potentials[v] - cost;
                calculated[u] = true;
            }
        }
    }

    {
        int64_t calculated_cnt = 0;
        for (auto is_calculated : calculated) {
            calculated_cnt += is_calculated;
        }
        assert(calculated_cnt == nodes.size());
    }

    return potentials;
}


void DFS(const std::vector<Edge>& edges, 
         const std::vector<Node>& nodes,
         const std::vector<std::vector<int64_t>>& graph,
         const std::set<int64_t>& basis_edges,
         int64_t vertex, int64_t parent, 
         std::vector<int64_t>& levels) {
    /*
        CHECK THIS CODE
    */
    int64_t current_level = 0;
    for (auto edge_index : graph[vertex]) {
        int64_t to = vertex ^ edges[edge_index].from ^ edges[edge_index].to;
        if (!basis_edges.contains(edge_index) || to == parent) {
            continue;
        }
        
        DFS(edges, nodes, graph, basis_edges, to, vertex, levels);
        current_level = std::max(current_level, levels[to] + 1);
    }
    levels[vertex] = current_level;
}


std::vector<int64_t> GetOptimalOrder(const std::vector<Edge>& edges, 
                                     const std::vector<Node>& nodes,
                                     const std::vector<std::vector<int64_t>>& graph,
                                     const std::set<int64_t>& basis_edges) {
                                    
    std::vector<int64_t> optimal_order(nodes.size());
    for (int64_t i = 0; i < int64_t{nodes.size()}; ++i) {
        optimal_order[i] = i;
    }
    std::vector<int64_t> levels(nodes.size());

    DFS(edges, nodes, graph, basis_edges, 0, -1, levels);

    std::sort(optimal_order.begin(), optimal_order.end(), [&levels](int64_t lhs, int64_t rhs) -> bool {
        return levels[lhs] < levels[rhs];
    });
    return optimal_order;
}


std::vector<int64_t> GetPseudoFlow(const std::vector<Edge>& edges, 
                                   const std::vector<Node>& nodes,
                                   const std::vector<std::vector<int64_t>>& graph,
                                   const std::set<int64_t>& basis_edges,
                                   const std::vector<int64_t>& potentials) {
    std::vector<int64_t> pseudo_flow(edges.size());
    std::vector<bool> already_calculated(edges.size(), false);

    for (int64_t edge_index = 0; edge_index < int64_t{edges.size()}; ++edge_index) {
        if (basis_edges.contains(edge_index)) { continue; }

        int64_t u = edges[edge_index].from;
        int64_t v = edges[edge_index].to;
        int64_t cost = edges[edge_index].cost;

        int64_t eval = (potentials[v] - potentials[u]) - cost;

        if (!eval) {
            std::cerr << "!!! dual_method.cpp/109/The problem is dually degenerate\n" << std::endl;
            // throw "The problem is dually degenerate\n";
        }

        if (eval <= 0) {
            pseudo_flow[edge_index] = edges[edge_index].low_limit;
        }
        if (eval > 0) {
            pseudo_flow[edge_index] = edges[edge_index].limit;
        }
        already_calculated[edge_index] = true;
    }

    std::vector<int64_t> order = std::move(GetOptimalOrder(edges, nodes, graph, basis_edges));

    for (auto node : order) {
        
        int64_t cnt_of_non_calculated = 0; // debug purposes

        for (auto edge_index : graph[node]) {
            if (!basis_edges.contains(edge_index) || already_calculated[edge_index]) {
                continue;
            }
            ++cnt_of_non_calculated;


            pseudo_flow[edge_index] = nodes[node].production;
            for (auto ei : graph[node]) {
                if (ei == edge_index) {
                    continue;
                }
                assert(already_calculated[ei]);

                if (edges[ei].from == node) {
                    pseudo_flow[edge_index] -= pseudo_flow[ei];
                }
                if (edges[ei].to == node) {
                    pseudo_flow[edge_index] += pseudo_flow[ei];
                }
            }

            if (edges[edge_index].to == node) {
                pseudo_flow[edge_index] *= -1;
            }

            already_calculated[edge_index] = true;
        }

        assert(cnt_of_non_calculated <= 1);
    }

    {
        int64_t calculated_cnt = 0;
        for (auto is_calculated : already_calculated) {
            calculated_cnt += is_calculated;
        }
        assert(calculated_cnt == edges.size());
    }

    return pseudo_flow;
}




std::vector<int64_t> DualMethod(const std::vector<Edge>& edges, 
                                const std::vector<Node>& nodes, 
                                const std::vector<std::vector<int64_t>>& graph,
                                std::set<int64_t>& basis_edges) {
    while (true) {
        auto potentials = std::move(GetPotentialsDualMethod(edges, nodes, basis_edges));
        auto pseudo_flow = std::move(GetPseudoFlow(edges, nodes, graph, basis_edges, potentials));

        int64_t not_optimal_edge_index = kNoneValue;
        int64_t not_optimal_value = kNoneValue;
        for (auto edge_index : basis_edges) {
            if (pseudo_flow[edge_index] < edges[edge_index].low_limit && 
                (not_optimal_edge_index == kNoneValue || not_optimal_value < edges[edge_index].low_limit - pseudo_flow[edge_index])) {
                not_optimal_edge_index = edge_index;
                not_optimal_value = edges[edge_index].low_limit - pseudo_flow[edge_index];
            }
            if (edges[edge_index].limit < pseudo_flow[edge_index] &&
                (not_optimal_edge_index == kNoneValue || not_optimal_value <  pseudo_flow[edge_index] - edges[edge_index].limit)) {
                not_optimal_edge_index = edge_index;
                not_optimal_value = pseudo_flow[edge_index] - edges[edge_index].limit;
            } 
        }
        if (not_optimal_edge_index == kNoneValue) {
            return pseudo_flow;
        }

        std::vector<Edge> edges_copy(edges);
        for (auto& edge : edges_copy) {
            edge.cost = 0;
        }
        if (pseudo_flow[not_optimal_edge_index] < edges[not_optimal_edge_index].low_limit) {
            edges_copy[not_optimal_edge_index].cost = -1;
        }
        if (edges[not_optimal_edge_index].limit < pseudo_flow[not_optimal_edge_index]) {
            edges_copy[not_optimal_edge_index].cost = 1;
        }
        auto l_values = std::move(GetPotentialsDualMethod(edges_copy, nodes, basis_edges));


        long double best_step = kNoneValue;
        int64_t best_step_edge_index = kNoneValue;

        for (int64_t ei = 0; ei < int64_t{edges.size()}; ++ei) {
            if (basis_edges.contains(ei)) { continue; }

            int64_t u = edges[ei].from;
            int64_t v = edges[ei].to;
            int64_t cost = edges[ei].cost;
            int64_t eval = (potentials[v] - potentials[u]) - cost;

            int64_t p_value = -(l_values[u] - l_values[v]);
            
            long double step = std::numeric_limits<long double>::max();
            if (eval * p_value < 0) {
                step = - (eval / p_value);
            }

            if (best_step == kNoneValue || step < best_step) {
                best_step = step;
                best_step_edge_index = ei;
            }
        }

        basis_edges.erase(not_optimal_edge_index);
        basis_edges.insert(best_step_edge_index);
    }
}