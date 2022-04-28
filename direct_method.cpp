#include "direct_method.h"


std::vector<int64_t> GetPotentials(const std::vector<Edge>& edges, 
                                   const std::vector<Node>& nodes,
                                   const std::set<int64_t>& basis_edges) {
    if (nodes.empty()) { 
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


int64_t GetNotOptimalEdgeIndex(const std::vector<Edge>& edges, 
                               const std::vector<Node>& nodes,
                               const std::set<int64_t>& basis_edges,
                               const std::vector<int64_t>& potentials,
                               const std::vector<int64_t>& flow) {
    int64_t ei_0 = kNoneValue;
    int64_t eval_0 = kNoneValue;
    for (int64_t edge_index = 0; edge_index < int64_t{edges.size()}; ++edge_index) {
        // std::cerr << edge_index << std::endl;
        if (basis_edges.contains(edge_index)) { continue; }

        int64_t u = edges[edge_index].from;
        int64_t v = edges[edge_index].to;
        int64_t cost = edges[edge_index].cost;
        int64_t eval = (potentials[v] - potentials[u]) - cost;

        // std::cerr << "edge : " << edge_index << " (" << u << " -> " << v << ") "  << " eval = " << potentials[v] << " - " << potentials[u] << " - " << cost << " = " << eval << std::endl;

        if ((eval <= 0 && flow[edge_index] == 0) ||
            (eval >= 0 && flow[edge_index] == edges[edge_index].limit)) {
            // std::cerr << "is optimal" << std::endl;
            continue;
        } else {
            // std::cerr << "is not optimal" << std::endl;
        }
        eval = abs(eval);
        if (ei_0 == kNoneValue || eval_0 < eval) {
            ei_0 = edge_index;
            eval_0 = eval;
        }
    }
    return ei_0;
}


void BuildCycle(const std::vector<Edge>& edges, 
                const std::vector<std::vector<int64_t>>& graph,
                int64_t vertex, int64_t parent, int64_t stop, 
                const std::set<int64_t>& basis_edges,
                std::vector<std::pair<int64_t, bool>>& cycle) {
    for (auto adjacent_edge_index : graph[vertex]) {
        int64_t next = vertex ^ edges[adjacent_edge_index].from ^ edges[adjacent_edge_index].to;

        if (!basis_edges.contains(adjacent_edge_index) || next == parent) {
            continue;
        }
        if (next == stop) {
            cycle.emplace_back(adjacent_edge_index, vertex == edges[adjacent_edge_index].from);
            return;
        }
        BuildCycle(edges, graph, next, vertex, stop, basis_edges, cycle);

        if (!cycle.empty()) {
            cycle.emplace_back(adjacent_edge_index, vertex == edges[adjacent_edge_index].from);
            return;
        }
    }

}


void Method(const std::vector<Edge>& edges, 
            const std::vector<Node>& nodes, 
            const std::vector<std::vector<int64_t>>& graph,
            std::vector<int64_t>& flow,
            std::set<int64_t>& basis_edges) {
    
    while (true) {
        std::cerr << "iteration" << std::endl;
        auto potentials = std::move(GetPotentials(edges, nodes, basis_edges));

        std::cerr << "potentials: " << std::endl;
        int pti = 0;
        for (auto potential : potentials) {
            std::cerr << (pti++) << " " << potential << std::endl;
        }
        std::cerr << std::endl;

        int64_t ei_0 = GetNotOptimalEdgeIndex(edges, nodes, basis_edges, potentials, flow);
        
        std::cerr << "edge with highest abs eval (from not optimal edges set): " << ei_0 << std::endl;

        if (ei_0 == kNoneValue) {
            break;
        }

        std::vector<std::pair<int64_t, bool>> cycle;
        if (flow[ei_0] == 0) {
            BuildCycle(edges, graph, edges[ei_0].to, edges[ei_0].from, edges[ei_0].from, basis_edges, cycle);
            cycle.emplace_back(ei_0, true);
        }
        if (flow[ei_0] == edges[ei_0].limit) {
            BuildCycle(edges, graph, edges[ei_0].from, edges[ei_0].to, edges[ei_0].to, basis_edges, cycle);
            cycle.emplace_back(ei_0, false);
        }
        std::reverse(cycle.begin(), cycle.end());


        std::vector<int64_t> thetta;
        for (const auto& [edge_index, is_straight] : cycle) {
            int64_t val;
            if (is_straight) {
                val = edges[edge_index].limit - flow[edge_index];
            } else {
                val = flow[edge_index];
            }
            thetta.push_back(val);
        }

        int64_t index = min_element(thetta.begin(), thetta.end()) - thetta.begin();

        int64_t min_thetta = thetta[index];
        int64_t min_thetta_edge_index = cycle[index].first;

        for (const auto& [edge_index, is_straight] : cycle) {
            if (is_straight) {
                flow[edge_index] += min_thetta;
            } else {
                flow[edge_index] -= min_thetta;
            }
        }

        if (min_thetta_edge_index != ei_0) {
            basis_edges.erase(min_thetta_edge_index);
            basis_edges.insert(ei_0);
        }
    }
}


std::pair<std::vector<int64_t>, std::set<int64_t>>
GetInitialFlow(const std::vector<Edge>& edges,
               const std::vector<Node>& nodes,
               const std::vector<std::vector<int64_t>>& graph) {
                   
    /* Building artificial network */
    std::vector<Edge> artificial_edges(edges);
    std::vector<Node> artificial_nodes(nodes);
    std::vector<std::vector<int64_t>> artificial_graph(graph);
    std::vector<int64_t> artificial_flow(edges.size(), 0);
    std::set<int64_t> basis_edges;

    for (auto& edge : artificial_edges) { edge.cost = 0; }

    int64_t artificial_node = nodes.size();
    artificial_nodes.push_back(Node{artificial_node, 0});
    artificial_graph.resize(artificial_nodes.size());
    for (auto node : nodes) {
        artificial_graph[node.vertex].push_back(artificial_edges.size());
        artificial_graph[artificial_node].push_back(artificial_edges.size());
        basis_edges.insert(artificial_edges.size());

        if (node.production >= 0) {
            artificial_edges.push_back(Edge{node.vertex, artificial_node, 1, node.production});
        } else {
            artificial_edges.push_back(Edge{artificial_node, node.vertex, 1, abs(node.production)});
        }
        artificial_flow.push_back(abs(node.production));
    }
    std::cerr << nodes.size() << std::endl;
    std::cerr << edges.size() << std::endl;

    std::cerr << artificial_nodes.size() << std::endl;
    std::cerr << artificial_edges.size() << std::endl;

    /* Solving first phase problem */
    Method(artificial_edges, artificial_nodes, artificial_graph, artificial_flow, basis_edges);

    /* Determining initial solution  */
    for (int64_t aei = edges.size(); aei < int64_t{artificial_edges.size()}; ++aei) {
        if (artificial_flow[aei]) {
            std::cerr << "direct_method.cpp/218/Network does not allow the flow." << std::endl;
            throw "No solution can be find.\n";
        }
    }
    
    //DON'T SURE
    //Добавляем натуральную дугу так, чтобы образовался цикл с двумя искусственными, одну искусственную удаляем
    for (int64_t ei = 0; ei < int64_t{edges.size()}; ++ei) {
        if (basis_edges.contains(ei)) {
            continue;
        }

        std::vector<std::pair<int64_t, bool>> cycle;
        BuildCycle(artificial_edges, artificial_graph, edges[ei].to, edges[ei].from, edges[ei].from, basis_edges, cycle);

        int64_t artificial_edges_in_cycle_cnt = 0;
        for (const auto& [ei_cycle, is_straight] : cycle) {
            if (ei_cycle < int64_t{edges.size()}) {
                continue;
            }
            ++artificial_edges_in_cycle_cnt;

            if (artificial_edges_in_cycle_cnt == 2) {
                basis_edges.erase(ei_cycle);
                basis_edges.insert(ei);
                break;
            }
        }
        assert(artificial_edges_in_cycle_cnt <= 2);
    }

    for (auto basis_edge : basis_edges) {
        if (edges.size() <= basis_edge && basis_edge < artificial_edges.size()) {
            basis_edges.erase(basis_edge);
            break;
        }
    }
    
    std::vector<int64_t> flow(std::make_move_iterator(artificial_flow.begin()), std::make_move_iterator(artificial_flow.begin() + edges.size()));

    assert(basis_edges.size() == (nodes.size() - 1));

    return {flow, basis_edges};
}


std::vector<int64_t> Solve(const std::vector<Edge>& edges,
                           const std::vector<Node>& nodes,
                           const std::vector<std::vector<int64_t>>& graph) {
    auto [flow, basis_edges] = std::move(GetInitialFlow(edges, nodes, graph));
    Method(edges, nodes, graph, flow, basis_edges);
    return flow;
}
