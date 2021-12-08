#include "SimpleObj.hpp"
#include <stdlib.h>
#include <string>

#include "logging.hpp"

std::vector<std::string> split(std::string_view input, std::string_view delim)
{
    std::vector<std::string> result = {};

    char* tok = strtok(const_cast<char*>(input.data()), (const char*)delim.data());
    while (tok)
    {
        result.emplace_back(tok);
        tok = strtok(nullptr, (const char*)delim.data());
    }

    if (result.size() == 0)
        result.emplace_back(input);

    return result;
}

namespace Scribble
{
    SimpleObj SimpleObj::Deserialize(std::ifstream& reader)
    {
        SimpleObj result;
        std::string line = "";
        while (std::getline(reader, line))
        {
            switch (line[0])
            {
            // this line contains a vertex pos or normal
            case 'v':
            {
                // if second char is an 'n' it's a normal, fuck those
                if (line[1] == ' ')
                {
                    // this is a vertex position
                    auto data = split(line, " ");
                    // z + 2 so the object gets loaded in front of you
                    result.vertices.emplace_back(-atof(data[1].c_str()), atof(data[2].c_str()), -atof(data[3].c_str()) + 2);
                }
            }
            break;
            // this line contains a face, but that just means it contains edges
            case 'f':
            {
                auto data = split(line, " ");
                int length = data.size();
                std::vector<int> indexes = {};
                for (int i = 1; i < length; i++)
                {
                    // vert / tex / normal indexes
                    auto set = split(data[i], "/");
                    // put all the vertex indexes into the one vector
                    indexes.emplace_back(atoi(set[0].c_str()) - 1);
                }
                length = indexes.size();
                for (int i = 0; i < length; i++)
                {
                    auto edge = Edge(indexes[i], indexes[(i + 1) % length]);

                    auto existing_pair = std::find_if(result.edges.begin(), result.edges.end(), [edge](auto existing)
                                                      { return edge == existing; });
                    if (existing_pair == result.edges.end())
                    {
                        // did not contain
                        result.edges.push_back(edge);
                    }
                }
            }
            break;
            default:
                break;
            }
        }
        return result;
    }
}