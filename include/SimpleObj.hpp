#pragma once
#include "sombrero/shared/Vector3Utils.hpp"

#include <fstream>
#include <utility>
#include <vector>

namespace Scribble
{
    struct Edge
    {
    public:
        Edge(int first, int second) : first(first), second(second) {}
        int first;
        int second;

        bool operator==(const Edge& other) const { return (this->first == other.first && this->second == other.second) || (this->second == other.first && this->first == other.second); }
    };

    class SimpleObj
    {
    public:
        static SimpleObj Deserialize(std::ifstream& reader);

        std::vector<Sombrero::FastVector3> vertices;
        std::vector<Edge> edges;
    };
}