#pragma once


#include <bits/stdc++.h>
#include "utility.h"


void Method(const std::vector<Edge>& edges, 
            const std::vector<Node>& nodes, 
            const std::vector<std::vector<int64_t>>& graph,
            std::vector<int64_t>& flow,
            std::set<int64_t>& basis_edges);


std::pair<std::vector<int64_t>, std::set<int64_t>>
GetInitialFlow(const std::vector<Edge>& edges,
               const std::vector<Node>& nodes,
               const std::vector<std::vector<int64_t>>& graph);


std::vector<int64_t> Solve(const std::vector<Edge>& edges,
                           const std::vector<Node>& nodes,
                           const std::vector<std::vector<int64_t>>& graph);
