#include "Seed.hpp"
#include "../Factories/FactorySelector/FactorySelector.hpp"
#include "../DeploymentManager/DeploymentManager.hpp"
#include <filesystem>

std::vector<std::shared_ptr<Node>> Seed::transferOwnership()
{
    std::vector<std::shared_ptr<Node>> allNodes;

    for (auto &node : listNode)
    {
        allNodes.push_back(std::move(node)); // std::move transfers ownership of the shared_ptr (from listNode to allNodes) without copying.
    }
    listNode.clear(); // Clear the seed's copy of the nodes
    return allNodes;
}


bool Seed::instantiateTopology(std::string topology)
{
    DeploymentManager deploymentManager(logger);

    if (deploymentManager.loadTopologyFromString(scenario,topology))
    {
        listNode = deploymentManager.getParsedNodes();
        logger.logInfo("Topology instantiated with "+std::to_string(listNode.size())+" nodes");

        return true;
    }

    return false;
}