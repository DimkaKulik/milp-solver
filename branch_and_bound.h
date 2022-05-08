#pragma once


#include "utility.h"
#include "direct_method.h"
#include "dual_method.h"
#include <bits/stdc++.h>


int64_t GetTargetFunctionValue(const std::vector<Edge>& edges,
                               const std::vector<int64_t>& flow, 
                               int64_t volume);


std::vector<int64_t> SolveMILP(const std::vector<Edge>& edges, 
                               const std::vector<Node>& nodes, 
                               const std::vector<std::vector<int64_t>>& graph,
                               int64_t volume);
