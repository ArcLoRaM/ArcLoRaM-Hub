#include "PhyLayer.hpp"


PhyLayer::PhyLayer( double distanceThreshold,Logger& logger) 
    : logger(logger),distanceThreshold(distanceThreshold) {

}


void PhyLayer::takeOwnership(std::vector<std::shared_ptr<Node>> nodes)
{
    for (auto& node : nodes) {
        registerNode(node);
    }

    // Calculate reachable nodes for each node (BASIC PHY LAYER)
   reachableNodesPerNode = getReachableNodesForAllNodes();

    for(std::shared_ptr<Node> node : nodes){
    
        Log initialNodeLog(node->initMessage(), true);
        logger.logMessage(initialNodeLog);

        node->setPhyLayer(this); // Set the PhyLayer for each node
    }



}

void PhyLayer::registerNode(std::shared_ptr<Node> node) {
    if (node == nullptr) {
    throw std::invalid_argument("Cannot register a nullptr node");
    }
    nodes.push_back(std::move(node));
}

// Calculate Euclidean distance between two nodes
double PhyLayer::calculateDistance(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
    double dx = a->getXCoordinate() - b->getXCoordinate();
    double dy = a->getYCoordinate() - b->getYCoordinate();
    return std::sqrt(dx * dx + dy * dy);
}

// Function to get pointers to nodes reachable from a given node
std::vector<std::shared_ptr<Node>> PhyLayer::getReachableNodesForNode(const std::shared_ptr<Node>& node) {
    std::vector<std::shared_ptr<Node>> reachable;
    for (const auto& other : nodes) {
        if (node != other && calculateDistance(node, other) <= distanceThreshold) {
            reachable.push_back(other);
        }
    }
    return reachable;
}

void PhyLayer::registerAllNodeEvents(Clock& clk) {
    for (const auto& ptrNode : nodes) {
        for (const auto& [activationTime, windowNodeState] : ptrNode->getActivationSchedule()) {
            clk.scheduleStateTransition(activationTime, [ptrNode, windowNodeState]() {
                ptrNode->onTimeChange(windowNodeState);
            });

            clk.scheduleCommunicationStep(activationTime, ptrNode);
        }
    }
}
// Function to get reachable nodes for all nodes
std::unordered_map<int,std::vector<std::shared_ptr<Node>>> PhyLayer::getReachableNodesForAllNodes() {
    std::unordered_map<int,std::vector<std::shared_ptr<Node>>> allReachableNodes;
    for (const auto& node : nodes) {
        if(allReachableNodes.find(node->getId()) != allReachableNodes.end()){
            throw std::runtime_error("Node ID " + std::to_string(node->getId()) + " already exists in the reachable nodes map.");
        }   
        allReachableNodes[node->getId()]= (getReachableNodesForNode(node));
    }
    logger.logMessage(Log("All reachable nodes calculated:", true));
    for (const auto& [key, value] : allReachableNodes) {
        std::string msg = "Node " + std::to_string(key) + " can reach: ";
        for (const auto& reachableNode : value) {
            msg += std::to_string(reachableNode->getId()) + " ";
        }
        logger.logMessage(Log(msg, true));
    }
    return allReachableNodes;
}





void PhyLayer::addTransmissionWindow(int senderId, int64_t start, int64_t end) {
    activeTransmissions.push_back({senderId, start, end});
}

void PhyLayer::removeTransmissionWindow(int senderId) {
    activeTransmissions.erase(
        std::remove_if(activeTransmissions.begin(), activeTransmissions.end(),
            [senderId](const TransmissionWindow& w) { return w.senderId == senderId; }),
        activeTransmissions.end());
}

bool PhyLayer::isReachable(int senderId, int receiverId) const {
    auto it = reachableNodesPerNode.find(senderId);
    if (it == reachableNodesPerNode.end()) return false;

    const auto& reachableList = it->second;
    return std::any_of(reachableList.begin(), reachableList.end(),
        [receiverId](const std::shared_ptr<Node>& n) { return n->getId() == receiverId; });
}

bool PhyLayer::isReceivingClean(const std::shared_ptr<Node>& receiver, int64_t currentTime) {
    int count = 0;
    int receiverId = receiver->getId();

    for (const auto& tx : activeTransmissions) {
        if (tx.start <= currentTime && tx.end > currentTime &&
            isReachable(tx.senderId, receiverId)) {
            ++count;
        }
    }

    return count == 1;
}

void PhyLayer::processTransmission(Node* sender, const std::vector<uint8_t>& message, int64_t airtimeMs) {
   
    if (!clock) throw std::runtime_error("Clock not set in PhyLayer.");

    int64_t start = clock->currentTimeInMilliseconds();
    int64_t end = start + airtimeMs;
    int64_t rxTime = start /*+ common::PROPAGATION_DELAY_MS*/; // Assuming no propagation delay for simplicity
    int senderId = sender->getId();

    clock->scheduleTransmissionStart(start, [this, senderId, start, end]() {
        logger.logMessage(Log("Transmission started by Node " + std::to_string(senderId), true));
        addTransmissionWindow(senderId, start, end);
    });

    for (const auto& receiver : reachableNodesPerNode[senderId]) {
        clock->scheduleReceiveEvent(rxTime, [this, receiver, message]() {
            int64_t currentTime = clock->currentTimeInMilliseconds();
            if (isReceivingClean(receiver, currentTime)) {
                receiver->receiveMessage(message);
            } else {
                receiver->markInterference();
            }
        });
    }

    clock->scheduleTransmissionEnd(end, [this, senderId]() {
        logger.logMessage(Log("Transmission ended by Node " + std::to_string(senderId), true));
        removeTransmissionWindow(senderId);
    });
    
}