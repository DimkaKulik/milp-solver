#include "algo.h"


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
        std::cerr << edge_index << std::endl;
        if (basis_edges.contains(edge_index)) { continue; }

        int64_t u = edges[edge_index].from;
        int64_t v = edges[edge_index].to;
        int64_t cost = edges[edge_index].cost;
        int64_t eval = (potentials[v] - potentials[u]) - cost;

        std::cerr << "edge : " << edge_index << " eval: " << eval << " ";

        if ((eval <= 0 && flow[edge_index] == 0) ||
            (eval >= 0 && flow[edge_index] == edges[edge_index].limit)) {
            std::cerr << "is optimal" << std::endl;
            continue;
        } else {
            std::cerr << "is not optimal" << std::endl;
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
            std::set<int64_t> basis_edges) {
    
    while (true) {
        std::cerr << "iteration" << std::endl;
        auto potentials = std::move(GetPotentials(edges, nodes, basis_edges));

        std::cerr << "potentials: " << std::endl;
        for (auto potential : potentials) {
            std::cerr << potential << " ";
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