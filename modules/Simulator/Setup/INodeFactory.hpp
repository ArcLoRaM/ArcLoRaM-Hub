#pragma once
#include "../Node/C1/C1_Node.hpp"
#include "../Node/C2/C2_Node.hpp"

#include "../Node/C3/C3_Node.hpp"

#include <memory>
#include <utility>

class INodeFactory {
public:
    virtual std::shared_ptr<C3_Node> createC3Node(int id, std::pair<int, int> coordinates) = 0;
    virtual std::shared_ptr<C2_Node> createC2Node(int id, std::pair<int, int> coordinates, int nextHop, int hopCount) = 0;
    virtual std::shared_ptr<C1_Node> createC1Node(int id, std::pair<int, int> coordinates) = 0;

    virtual ~INodeFactory() = default;
};
