#include "branch_and_bound.h"


// int64_t GetTargetFunctionValue(const std::vector<Edge>& edges,
//                                const std::vector<int64_t>& flow, 
//                                int64_t volume) {
//     int64_t value = 0;
//     for (int64_t i = 0; i < int64_t{edges.size()}; ++i) {
//         int64_t cost = edges[i].cost;
//         int64_t cars = (flow[i] + volume - 1) / volume;
//         value += cost * cars; 
//     }
//     return value;
// }


int64_t GetTargetFunctionValue(const std::vector<Edge>& edges,
                                   const std::vector<int64_t>& flow, 
                                   int64_t volume) {
    int64_t value = 0;
    for (int64_t i = 0; i < int64_t{edges.size()}; ++i) {
        int64_t cost = edges[i].cost;
        int64_t cars = (flow[i] + volume - 1) / volume;
        value += cost * cars; 
    }
    return value;
}

std::vector<int64_t> BranchAndBound(const std::vector<Edge>& edges, 
                                    const std::vector<Node>& nodes, 
                                    const std::vector<std::vector<int64_t>>& graph,
                                    const std::vector<int64_t> flow,
                                    const std::set<int64_t>& basis_edges,
                                    int64_t volume) {
    std::vector<int64_t> best_flow(flow);
    std::cerr << GetTargetFunctionValue(edges, flow, volume) << std::endl;
    for (int64_t i = 0; i < int64_t{flow.size()}; ++i) {
        if (flow[i] % volume == 0) {
            continue;
        }

        std::vector<Edge> left_branch_edges(edges);
        std::set<int64_t> left_branch_basis(basis_edges);
        left_branch_edges[i].low_limit = (flow[i] / volume + 1) * volume;
        if (left_branch_edges[i].limit < left_branch_edges[i].low_limit) {
            continue;
        }
        auto left_branch_flow = DualMethod(left_branch_edges, nodes, graph, left_branch_basis);

        std::vector<Edge> right_branch_edges(edges);
        std::set<int64_t> right_branch_basis(basis_edges);
        right_branch_edges[i].limit = (flow[i] / volume) * volume;
        auto right_branch_flow = DualMethod(right_branch_edges, nodes, graph, right_branch_basis);

        
        int64_t left_branch_eval = GetTargetFunctionValue(edges, left_branch_flow, volume);
        int64_t center_branch_eval = GetTargetFunctionValue(edges, flow, volume);
        int64_t right_branch_eval = GetTargetFunctionValue(edges, right_branch_flow, volume);

        if (left_branch_eval < center_branch_eval && left_branch_eval < right_branch_eval) {
            // std::cerr << "Branch and Bound started on " << i + 1 << " component" << std::endl;
            // std::cerr << left_branch_edges[i].low_limit << " " << left_branch_edges[i].limit << std::endl;
            best_flow = BranchAndBound(left_branch_edges, nodes, graph, left_branch_flow, left_branch_basis, volume);
        }

        if (center_branch_eval < left_branch_eval && center_branch_eval < right_branch_eval) {
            // std::cerr << "Branch and Bound started on " << i + 1 << " component" << std::endl;
            // std::cerr << left_branch_edges[i].low_limit << " " << left_branch_edges[i].limit << std::endl;
            best_flow = flow;
            // best_flow = BranchAndBound(edges, nodes, graph, flow, basis_edges, volume);
        }

        if (right_branch_eval < left_branch_eval && right_branch_eval < center_branch_eval) {
            // std::cerr << "Branch and Bound started on " << i + 1 << " component" << std::endl;
            // std::cerr << left_branch_edges[i].low_limit << " " << left_branch_edges[i].limit << std::endl;

            best_flow = BranchAndBound(right_branch_edges, nodes, graph, right_branch_flow, right_branch_basis, volume);
        }
    }
    std::cerr << "result eval of branch and bound: " << GetTargetFunctionValue(edges, best_flow, volume) << std::endl;
    return best_flow;
}


std::vector<int64_t> SolveMILP(const std::vector<Edge>& edges, 
                               const std::vector<Node>& nodes, 
                               const std::vector<std::vector<int64_t>>& graph,
                               int64_t volume) {
    auto [initial_flow, basis_edges] = std::move(GetInitialFlow(edges, nodes, graph));
    return BranchAndBound(edges, nodes, graph, initial_flow, basis_edges, volume);
}
