#pragma once


#include <bits/stdc++.h>
#include "utility.h"


std::vector<int64_t> DualMethod(const std::vector<Edge>& edges, 
                                const std::vector<Node>& nodes, 
                                const std::vector<std::vector<int64_t>>& graph,
                                std::set<int64_t>& basis_edges);