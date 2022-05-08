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

        if (calculated_cnt != nodes.size()) {
            for (auto edge_index : basis_edges) {
                std::cerr << edges[edge_index].from + 1 << " " << edges[edge_index].to + 1 << std::endl;
            }
            std::cerr << calculated_cnt << " " << nodes.size() << std::endl;
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
        // std::cerr << "eval of (" << edges[edge_index].from + 1 << "->" << edges[edge_index].to + 1 << "): " << eval << std::endl;

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


void VisitSomeNodes(const std::vector<Edge>& edges, 
                    const std::vector<Node>& nodes,
                    const std::vector<std::vector<int64_t>>& graph,
                    const std::set<int64_t>& basis_edges,
                    std::vector<bool>& visited,
                    int64_t vertex) {
    if (visited[vertex]) {
        return;
    } else {
        visited[vertex] = true;
    }

    for (auto edge_index : graph[vertex]) {
        int64_t to = vertex ^ edges[edge_index].from ^ edges[edge_index].to;
        if (!basis_edges.contains(edge_index) || visited[to]) {
            continue;
        }
        VisitSomeNodes(edges, nodes, graph, basis_edges, visited, to);   
    }
}



void UpdateBasisEdgesSet(const std::vector<Edge>& edges, 
                         const std::vector<Node>& nodes, 
                         const std::vector<std::vector<int64_t>>& graph, 
                         std::set<int64_t>& basis_edges, 
                         const std::vector<int64_t>& candidates, 
                         int64_t to_delete) {
    for (auto candidate : candidates) {
        basis_edges.erase(to_delete);
        basis_edges.insert(candidate);

        std::vector<bool> visited(nodes.size(), false);
        
        VisitSomeNodes(edges, nodes, graph, basis_edges, visited, 0);
        
        bool all_nodes_visited = true;
        for (auto used : visited) {
            if (!used) {
                all_nodes_visited = false;
            }
        }

        if (all_nodes_visited) {
            return;
        } else {
            basis_edges.erase(candidate);
            basis_edges.insert(to_delete);
        }
    }

    std::cerr << "FATAL ERROR, CANNOT ADD ANY EDGE TO MAKE BASIS SET TREE-LIKE" << std::endl;
    throw "exception";
}




std::vector<int64_t> DualMethod(const std::vector<Edge>& edges, 
                                const std::vector<Node>& nodes, 
                                const std::vector<std::vector<int64_t>>& graph,
                                std::set<int64_t>& basis_edges) {
    std::cerr << "DUAL METHOD STARTS" << std::endl;
    int64_t iterations = 0;
    while (true) {
        ++iterations;
        std::cerr << "iteration: ";
        std::cerr << iterations << std::endl;

        // std::cerr << "basis edges: " << std::endl;
        // for (auto edge_index : basis_edges) {
        //     std::cerr << edges[edge_index].from + 1 << " " << edges[edge_index].to + 1 << std::endl;
        // }



        auto potentials = std::move(GetPotentialsDualMethod(edges, nodes, basis_edges));
        // std::cerr << "potentials" << std::endl;
        // for (auto pot : potentials) {
        //     std::cerr << pot << " ";
        // }
        // std::cerr << std::endl;
        auto pseudo_flow = std::move(GetPseudoFlow(edges, nodes, graph, basis_edges, potentials));
        // std::cerr << "pseudo flow" << std::endl;
        // for (int64_t edge_index = 0; edge_index < edges.size(); ++edge_index) {
        //     std::cerr << edges[edge_index].from + 1 << "->" << edges[edge_index].to + 1 << " pseudo flow is " << pseudo_flow[edge_index] << std::endl;
        // }


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

        // std::cerr << "L:" << std::endl;
        // for (auto x : l_values) {
        //     std::cerr << x << " ";
        // }
        // std::cerr << std::endl;

        int64_t best_step = std::numeric_limits<int64_t>::max();
        std::vector<int64_t> candidates;

        for (int64_t ei = 0; ei < int64_t{edges.size()}; ++ei) {
            if (basis_edges.contains(ei)) { continue; }

            int64_t u = edges[ei].from;
            int64_t v = edges[ei].to;
            int64_t cost = edges[ei].cost;
            int64_t eval = (potentials[v] - potentials[u]) - cost;

            int64_t p_value = -(l_values[u] - l_values[v]);
            // std::cerr << "p val " << edges[ei].from + 1 << " " << edges[ei].to + 1 << " " << p_value << std::endl;
            assert(p_value == -1 || p_value == 1 || p_value == 0);

            int64_t step = std::numeric_limits<int64_t>::max();
            if (eval * p_value < 0) {
                step = - (eval / p_value);
            }

            if (step < best_step) {
                best_step = step;
                candidates.clear();
                candidates.push_back(ei);
            } else if (step == best_step) {
                candidates.push_back(ei);
            }
        }
        std::random_shuffle(candidates.begin(), candidates.end());
        // if (best_step_edge_index == -1) {
        //     throw "asds";
        // }

        // std::cerr << "before" << std::endl;
        // for (auto edge_index : basis_edges) {
        //     std::cerr << edges[edge_index].from + 1<< " " << edges[edge_index].to + 1 << std::endl;
        // }

        // std::cerr << "Candidates size: " << candidates.size() << std::endl;
        if (best_step == std::numeric_limits<int64_t>::max()) {
            break;
        }
        UpdateBasisEdgesSet(edges, nodes, graph, basis_edges, candidates, not_optimal_edge_index);
        // basis_edges.erase(not_optimal_edge_index);
        // basis_edges.insert(best_step_edge_index);
        // std::cerr << "after" << std::endl;
        // for (auto edge_index : basis_edges) {
        //     std::cerr << edges[edge_index].from  + 1<< " " << edges[edge_index].to + 1<< std::endl;
        // }
        // std::cerr << "*********************" << std::endl;
    }


    std::vector<int64_t> flow(edges.size());
    for (int64_t i = 0; i < int64_t{edges.size()}; ++i) {
        flow[i] = edges[i].limit;
    }
    return flow;
}