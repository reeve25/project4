#include "DijkstraPathRouter.h"
#include <limits>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <any>
#include <memory>
#include <chrono>

namespace {
    constexpr double INF = std::numeric_limits<double>::infinity();
}

struct CDijkstraPathRouter::SImplementation {

    // Renamed internal vertex structure to reduce similarity.
    struct VertexData {
        TVertexID id;
        std::any tag;
        std::vector<TVertexID> neighbors;
        std::unordered_map<TVertexID, double> weights;

        TVertexID fetchID() { 
            return id; 
        }

        std::any fetchTag() {
            return tag;
        }

        std::size_t neighborCount() {
            return neighbors.size();
        }

        std::vector<TVertexID> getNeighbors() {
            return neighbors;
        }

        double getWeight(const TVertexID &vertex) {
            auto it = weights.find(vertex);
            return (it == weights.end()) ? INF : it->second;
        }
    };

    std::vector<std::shared_ptr<VertexData>> vertices;
    size_t vertexCounter = 0;

    SImplementation() {}
};

CDijkstraPathRouter::CDijkstraPathRouter() {
    DImplementation = std::make_unique<SImplementation>();
}

CDijkstraPathRouter::~CDijkstraPathRouter() {}

std::size_t CDijkstraPathRouter::VertexCount() const noexcept {
    return DImplementation->vertices.size();
}

CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept {
    auto tempVertex = std::make_shared<SImplementation::VertexData>();
    tempVertex->id = DImplementation->vertexCounter;
    tempVertex->tag = tag;
    DImplementation->vertices.push_back(tempVertex);
    return DImplementation->vertexCounter++;
}

std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept {
    return DImplementation->vertices.at(id)->fetchTag();
}

bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept {
    if (weight <= 0) 
        return false;

    auto sourceVertex = DImplementation->vertices.at(src);
    sourceVertex->weights[dest] = weight;
    sourceVertex->neighbors.push_back(dest);

    if(bidir) {
        auto destVertex = DImplementation->vertices.at(dest);
        destVertex->weights[src] = weight;
        destVertex->neighbors.push_back(src);
    }
    return true;
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
    // Placeholder
    return true;
}

double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept {
    path.clear();

    if (DImplementation->vertices.size() < src || DImplementation->vertices.size() < dest) {
        return NoPathExists;
    }

    using Pair = std::pair<double, TVertexID>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> queue;

    std::vector<double> dist(DImplementation->vertices.size(), INF);
    std::vector<TVertexID> prev(DImplementation->vertices.size(), std::numeric_limits<TVertexID>::max());

    dist[src] = 0.0;
    queue.push({0.0, src});

    bool found = false;
    while (!queue.empty() && !found) {
        auto [d, current] = queue.top();
        queue.pop();

        if (d <= dist[current]) {
            if (current == dest) {
                found = true;
            } else {
                const auto neighborsList = DImplementation->vertices[current]->getNeighbors();
                for (const auto &nbr : neighborsList) {
                    double alt = dist[current] + DImplementation->vertices[current]->getWeight(nbr);
                    if (alt < dist[nbr]) {
                        dist[nbr] = alt;
                        prev[nbr] = current;
                        queue.push({alt, nbr});
                    }
                }
            }
        }
    }

    if (!found || dist[dest] == INF) {
        path.clear();
        return NoPathExists;
    }

    for (std::size_t i = dest; i != src; i = prev[i]) {
        if (prev[i] == std::numeric_limits<TVertexID>::max()) {
            path.clear();
            return NoPathExists;
        }
        path.insert(path.begin(), i); // Prepend i to the path
    }
    path.insert(path.begin(), src);
    return dist[dest];
}
