#include "Seed.hpp"
#include "../Factories/RrcUplinkNodeFactory/RrcUplinkNodeFactory.hpp"
#include "../Factories/FactorySelector/FactorySelector.hpp"
#include "../DeploymentManager/DeploymentManager.hpp"
#include <filesystem>
// we don't adopt the real time windows for the moment, as it is really impractical (a few dowens of ms every minutes...)

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

void Seed::initializeNodes()
{
    std::string use_case = communicationWindow + "_" + topology;

#if COMMUNICATION_PERIOD == RRC_DOWNLINK || COMMUNICATION_PERIOD == RRC_BEACON

    if (use_case == "RRC_Beacon_Line")
    {
        initialize_RRC_Beacon_Line();
    }
    else if (use_case == "RRC_Beacon_Mesh")
    {
        initialize_RRC_Beacon_Mesh();
    }
    else if (use_case == "RRC_Downlink_Line")
    {
        initialize_RRC_Downlink_Line();
    }
    else if (use_case == "RRC_Downlink_Mesh")
    {
        Log log("RRC_Downlink_Mesh runnning", true);
        logger.logMessage(log);
        initialize_RRC_Beacon_Mesh(); // Provisionning is the same, TDMA may differ, if protocol is fully implemented
    }
    else if (use_case == "RRC_Beacon_Mesh_Self_Healing")
    {
        initialize_RRC_Beacon_Mesh_Self_Healing();
    }
#elif COMMUNICATION_PERIOD == RRC_UPLINK
    if (use_case == "RRC_Uplink_Mesh")
    {
        if(common::readConfigFromFile){
            initialize_RRC_Uplink_Mesh_FromFile();
        }
        else{
            initialize_RRC_Uplink_Mesh();
        }
    }
    else if (use_case == "RRC_Uplink_Line")
    {
        if(common::readConfigFromFile){
            initialize_RRC_Uplink_Line_FromFile();
        }
        else{
            initialize_RRC_Uplink_Line();
        }
        
    }
#endif
}
//it's the same actually from a provisionning point of view
#if COMMUNICATION_PERIOD == RRC_DOWNLINK || COMMUNICATION_PERIOD == RRC_BEACON

void Seed::initialize_RRC_Beacon_Mesh()
{
    /*                 -- C2--
                   ---        ---
             ---C2-----------------C2
            ---        ---      ---
        C3                --C2--
          ---          ---     --
             ---C2---            --C2

    *                 ----4 ----
                   ---        ---
             --- 1----------------- 5
            ---       ---      ---
         0               -- 3--
          ---         ---     --
             --- 2---            -- 6
    */

    // create a C3 node
    std::pair<int, int> coordinates = std::make_pair(0, 0);
    auto firstNode = std::make_shared<C3_Node>(0, logger, coordinates, dispatchCv, dispatchCvMutex);

    for (size_t i = 0; i < common::nbComWindows; i++)
    {
        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanTransmit); // the C3 is only transmitting in this mode

        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
    }
    listNode.push_back(firstNode);

    // Create C2 nodes in a mesh configuration
    int nbC2Nodes = 6;
    std::vector<std::pair<int, int>> coordinatesC2 = {std::make_pair(600, 600), std::make_pair(600, -600), std::make_pair(1200, 0),
                                                      std::make_pair(1200, 1200), std::make_pair(1800, 600), std::make_pair(1800, -600)};
        Log log1("Nodes will create", true);
        logger.logMessage(log1);
    for (int i = 1; i < nbC2Nodes + 1; i++)
    {
        std::shared_ptr<C2_Node> node;
        if (i == 1) // to prove energy aware Routing works
        {   
            node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 10.0); // Create a smart pointer
        }
        else
        {
            node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 80.0); // Create a smart pointer
        }

        for (size_t i = 0; i < common::nbComWindows; i++)
        {
            node->addActivation((i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanCommunicate); // the C3 is only transmitting in this mode

            node->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
        }
        listNode.push_back(node);
    }
    Log log2("Nodes are created", true);
    logger.logMessage(log2);
}

void Seed::initialize_RRC_Beacon_Mesh_Self_Healing()
{

    /*
                   C2    -----   C2---
                    -                  ---
                     -                    C2
                     -               ---
        C3 --------  C2 -------- C2



                    2   -----   4---
                    -                  ---
                     -                    5
                     -               ---
        0 --------  1 -------- 3

        3 is dead -> no activation schedula planned, the network should self-heal and reroute 6 through 5 and 3
    */

    // create a C3 node
    std::pair<int, int> coordinates = std::make_pair(-300, -600);
    auto firstNode = std::make_shared<C3_Node>(0, logger, coordinates, dispatchCv, dispatchCvMutex);

    for (size_t i = 0; i < common::nbComWindows; i++)
    {
        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanTransmit); // the C3 is only transmitting in this mode

        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
    }
    listNode.push_back(firstNode);

    int nbC2Nodes = 5;
    std::vector<std::pair<int, int>> coordinatesC2 =
        {std::make_pair(600, -600), std::make_pair(800, 200), std::make_pair(1700, -600),
         std::make_pair(1700, 600), std::make_pair(2200, 0)};

    for (int i = 1; i < nbC2Nodes + 1; i++)
    {
        std::shared_ptr<C2_Node> node;
        if (i == 3) // this node will be dead
        {
            node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 0.0, 1,2); // Create a smart pointer
        }
        else
        {

            if (i == 2)
            {
                node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 60.0, 1,2); // Create a smart pointer
            }
            else if (i == 4)
            {
                node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 60.0, 2,3); // Create a smart pointer
            }
            else if (i == 5)
            {
                node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 60.0, 3,3); // Create a smart pointer
            }
            else if (i == 1)
            {
                node = std::make_shared<C2_Node>(i, logger, coordinatesC2[i - 1], dispatchCv, dispatchCvMutex, 60.0, 0,1); // Create a smart pointer
            }
            for (size_t i = 0; i < common::nbComWindows; i++)
            {
                node->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanCommunicate); // the C3 is only transmitting in this mode

                node->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
            }
        }

        listNode.push_back(node);
    }
}

void Seed::initialize_RRC_Beacon_Line()
{

    /*

        C3 --------  C2 -------- C2 -------- C2 -------- C2

    */

    // create a C3 node
    std::pair<int, int> coordinates = std::make_pair(0, 0);
    auto firstNode = std::make_shared<C3_Node>(0, logger, coordinates, dispatchCv, dispatchCvMutex);

    for (size_t i = 0; i < common::nbComWindows; i++)
    {
        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanTransmit); // the C3 is only transmitting in this mode

        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
    }

    listNode.push_back(firstNode);

    // Create C2 nodes in a line
    for (int i = 1; i < 5; i++)
    {
        std::pair<int, int> coordinate = std::make_pair(800 * i, 0);
        auto node = std::make_shared<C2_Node>(i, logger, coordinate, dispatchCv, dispatchCvMutex, 80); // Create a smart pointer

        for (size_t i = 0; i < common::nbComWindows; i++)
        {
            node->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanCommunicate); // the C3 is only transmitting in this mode

            node->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
        }
        listNode.push_back(node);
    }
}

void Seed::initialize_RRC_Downlink_Line()
{

    /*

        C3 --------  C2 -------- C2 -------- C2 -------- C2

    */

    // create a C3 node
    std::pair<int, int> coordinates = std::make_pair(0, 0);
    auto firstNode = std::make_shared<C3_Node>(0, logger, coordinates, dispatchCv, dispatchCvMutex);

    for (size_t i = 0; i < common::nbComWindows; i++)
    {
        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanTransmit); // the C3 is only transmitting in this mode
        firstNode->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
    }

    listNode.push_back(firstNode);

    // Create C2 nodes in a line
    for (int i = 1; i < 5; i++)
    {
        std::pair<int, int> coordinate = std::make_pair(800 * i, 0);
        auto node = std::make_shared<C2_Node>(i, logger, coordinate, dispatchCv, dispatchCvMutex, 80.0); // Create a smart pointer

        for (size_t i = 0; i < common::nbComWindows; i++)
        {
            node->addActivation( (i + 1) * common::lengthSleepingWindow + i * common::lengthTransmissionWindow, WindowNodeState::CanCommunicate); // the C3 is only transmitting in this mode
            node->addActivation( (i + 1) * common::lengthSleepingWindow + (i + 1) * common::lengthTransmissionWindow, WindowNodeState::CanSleep);
        }
        listNode.push_back(node);
    }
}
#elif COMMUNICATION_PERIOD == RRC_UPLINK
void Seed::initialize_RRC_Uplink_Mesh()
{
    /*              -- C2--
                ---        ---
          ---C2-----------------C2
         ---        ---      ---
     C3                --C2--
       ---          ---     --
          ---C2---            --C2

 *                 ----4 ----
                ---        ---
          --- 1----------------- 5
         ---       ---      ---
      0               -- 3--
       ---         ---     --
          --- 2---            -- 6

     Odd:1,2,5,6
     Even:3,4
 */




    auto factory = FactorySelector::getFactory(common::getCurrentCommunicationMode(),logger);

    // Create the C3 node
    auto c3Node = factory->createC3Node(0, {0, 0});
    listNode.push_back(c3Node);

    // Define C2 node parameters
    std::vector<C2_Node_Mesh_Parameter> C2_Parameters = {
        {{600, 600}, 1, 0},
        {{600, -600}, 1, 0},
        {{1200, 0}, 2, 1},
        {{1200, 1200}, 2, 1},
        {{1800, 600}, 3, 4},
        {{1800, -600}, 3, 3}
    };

    // Create and add C2 nodes via factory
    for (int i = 1; i <= static_cast<int>(C2_Parameters.size()); i++)
    {
        const auto& param = C2_Parameters[i - 1];
        auto node = factory->createC2Node(i, param.coordinates, param.nextNodeIdInPath, param.hopCount);
        listNode.push_back(node);
    }


}


void Seed::initialize_RRC_Uplink_Mesh_FromFile()
{
    DeploymentManager deploymentManager(logger);

    
    std::string filePath="";
    for (const auto& entry : std::filesystem::directory_iterator("Setup/config/")) {
        if (entry.is_regular_file() && entry.path().extension() == ".simcfg") {
            filePath= entry.path().string(); // full path
        }
    }

    if(filePath == "") {
        throw std::runtime_error(std::string("No .simcfg file found in Setup/config/"));
    }
    // Load from the deployment file
    auto nodes = deploymentManager.loadDeploymentFromFile(filePath);
    // Take ownership of the nodes
    listNode = std::move(nodes);
    Log fileReadingLog("Deployment loaded from file: deployment_mesh.simcfg", true);
    logger.logMessage(fileReadingLog);
}

void Seed::initialize_RRC_Uplink_Line()
{
    /*

        C3 --------  C2 -------- C2 -------- C2 -------- C2

    */

    //should use the factory selector
RrcUplinkNodeFactory factory(logger);

    // Create the C3 node
    auto firstNode = factory.createC3Node(0, {0, 0});
    listNode.push_back(firstNode);


    // Create C2 nodes in a line
    int hopCount = 1;
    for (int i = 1; i < 5; i++)
    {
        auto coordinate = std::make_pair(800 * i, 0);

        // Use factory (mode-specific TDMA setup inside factory)
        auto node = factory.createC2Node(i, coordinate, i - 1, hopCount);
        listNode.push_back(node);

        hopCount++;
    }



    //old stuff to erase
    // std::pair<int, int> coordinates = std::make_pair(0, 0);
    // auto firstNode = std::make_shared<C3_Node>(0, logger, coordinates, dispatchCv, dispatchCvMutex);

    // for (size_t i = 0; i < common::totalNumberOfSlots; i++)
    // { // initially sleep
    //     firstNode->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + i * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanListen);
    //     firstNode->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanSleep);
    //     firstNode->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanTransmit);
    //     firstNode->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + (i + 1) * common::durationACKWindow, WindowNodeState::CanSleep);
    // }

    // listNode.push_back(firstNode);

    // // Create C2 nodes in a line
    // int hopCount = 1;
    // for (int i = 1; i < 5; i++)
    // {
    //     std::pair<int, int> coordinate = std::make_pair(800 * i, 0);

    //     // Second COnstructor for C2 (as if Beacon Mode has already provided necessary information)
    //     auto node = std::make_shared<C2_Node>(i, logger, coordinate, dispatchCv, dispatchCvMutex, i - 1, hopCount); // Create a smart pointer
    //     hopCount++;
    //     for (size_t i = 0; i < common::totalNumberOfSlots; i++)
    //     {
    //         node->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + i * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanCommunicate);
    //         node->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanSleep);
    //         node->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanCommunicate);
    //         node->addActivation(baseTime + (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + (i + 1) * common::durationACKWindow, WindowNodeState::CanSleep);
    //     }
    //     listNode.push_back(node);
    // }
}



void Seed::initialize_RRC_Uplink_Line_FromFile()
{
    DeploymentManager deploymentManager(logger);

    // Load from the deployment file
    auto nodes = deploymentManager.loadDeploymentFromFile("Setup/config/deployment_line.simcfg");

    // Take ownership of the nodes
    listNode = std::move(nodes);
    Log fileReadingLog("Deployment loaded from file: deployment_line.simcfg", true);
    logger.logMessage(fileReadingLog);
}
#endif