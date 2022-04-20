#include "utility.h"


void ReadGraph(const std::string& edges_filename, 
               const std::string& nodes_filename, 
               std::vector<Edge>* edges, 
               std::vector<Node>* nodes, 
               std::vector<std::vector<int64_t>>* graph) {
    std::ifstream edges_file(edges_filename);
    int64_t edges_records_count;
    edges_file >> edges_records_count;
    for (int64_t i = 0; i < edges_records_count; ++i) {
        int64_t from, to, cost, limit;
        edges_file >> from >> to >> cost >> limit;

        if (int64_t{graph->size()} <= std::max(from, to)) {
            graph->resize(std::max(from, to) + 1);
        }

        (*graph)[from].push_back(i);
        (*graph)[to].push_back(i);

        edges->push_back(Edge{from, to, cost, limit});
    }
    edges_file.close();
    
    std::ifstream nodes_file(nodes_filename);
    int64_t nodes_records_count;
    nodes_file >> nodes_records_count;

    nodes->resize(graph->size());
    for (int64_t i = 0; i < nodes_records_count; ++i) {
        int64_t vertex, production;
        nodes_file >> vertex >> production;

        if (int64_t{nodes->size()} <= vertex) {
            throw "Do not use isolated vertices in the input data.\n";
        }

        (*nodes)[vertex] = Node{vertex, production};
    }
    nodes_file.close();
}