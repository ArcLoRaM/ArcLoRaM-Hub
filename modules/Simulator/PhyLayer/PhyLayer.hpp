#pragma once



#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include "../Node/Node.hpp" 
#include "../Clock/Clock.hpp"

class PhyLayer {
public:
    PhyLayer(double distanceThreshold,Logger& logger);

    void takeOwnership(std::vector<std::shared_ptr<Node>> nodes);

    int getNbNodes() const { return nodes.size(); };

    void processTransmission(std::shared_ptr<Node> sender, const std::vector<uint8_t>& message, int64_t airtimeMs);


    // Function to get reachable nodes for a specific node
    std::vector<std::shared_ptr<Node>> getReachableNodesForNode(const std::shared_ptr<Node>& node);
void registerAllNodeEvents(Clock& clk);


private:

    struct TransmissionWindow {
        int senderId;
        int64_t start;
        int64_t end;
    };

    std::vector<TransmissionWindow> activeTransmissions;

    Clock* clock = nullptr; //to schedule the transmission and reception of messages

    void registerNode(std::shared_ptr<Node> node);

    std::unordered_map<int, std::vector<std::shared_ptr<Node>>> reachableNodesPerNode; // Map for quick access to reachable nodes by node ID
    bool checkForMessages();
    double distanceThreshold;
    // Helper function to calculate Euclidean distance between two nodes
    double calculateDistance(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);

  Logger& logger;


      // Function to get reachable nodes for all nodes
    std::unordered_map<int,std::vector<std::shared_ptr<Node>>> getReachableNodesForAllNodes();
    std::vector<std::shared_ptr<Node>> nodes;//heterogeneous container of nodes (C1, C2...)


    void addTransmissionWindow(int senderId, int64_t start, int64_t end);
    void removeTransmissionWindow(int senderId);
    bool isReceivingClean(const std::shared_ptr<Node>& receiver, int64_t currentTime);
    bool isReachable(int senderId, int receiverId) const;
};
