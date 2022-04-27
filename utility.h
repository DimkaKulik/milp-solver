#pragma once


#include <bits/stdc++.h>


const int64_t kNoneValue = -1;


struct Edge {
    int64_t from;
    int64_t to;
    int64_t cost;
    int64_t limit;
    
    int64_t low_limit = 0;
};


struct Node {
    int64_t vertex;
    int64_t production;
};


void ReadGraph(const std::string& edges_filename, 
               const std::string& nodes_filename, 
               std::vector<Edge>* edges, 
               std::vector<Node>* nodes, 
               std::vector<std::vector<int64_t>>* graph);
