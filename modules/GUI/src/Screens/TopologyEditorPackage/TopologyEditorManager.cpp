#include "TopologyEditorManager.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"

TopologyEditorManager::TopologyEditorManager(TopologyEditorState &state)
    : nodeCounter(0), selectedNodeId(std::nullopt),
      modeDropdown(180.f, 30.f), state(state)
{
    coordText.emplace(ResourceManager::getInstance().getFont("Arial"));
    coordText->setCharacterSize(26);
    coordText->setFillColor(sf::Color::White);
    coordText->setPosition({500.f, 20.f});

    topologyBounds.setFillColor(sf::Color::Transparent);
    topologyBounds.setOutlineColor(sf::Color::Green);
    topologyBounds.setOutlineThickness(4.f);

    modeDropdown.setPosition({200.f, 40.f});
    modeDropdown.setCallback([this, &state](TopologyMode mode)
                             {
        state.setTopologyMode(mode);
        std::cout << "Selected mode: " << static_cast<int>(mode) << std::endl; });

    // create SAVE + C1,2,3 Add Nodes + Routing Button:

    const sf::Color color = sf::Color::White;

    constexpr float buttonWidth = 100.f;
    constexpr float buttonHeight = 100.f;
    constexpr float spacingY = 20.f;
    constexpr float startX = 20.f;
    float posY = 20.f;
    posY +=  40.f+spacingY;

    saveButton = std::make_unique<Button>(
        startX,
        posY,
        130.f,
        50.f,
        sf::Color(50, 200, 50),
        "Save",
        "Arial",
        false);
    saveButton->setOnClick([this]()
                           {
                               // TODO: Save functionality
                           });

    posY += buttonHeight + spacingY;
    addNodeC1Button = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 150, 250),
        "C1",
        "Arial",
        true);
    addNodeC1Button->setOnClick([this, &state]()
                                {
                                    // state.setEditorMode(EditorMode::AddingC1Node);
                                    // addNodeC3Button->depush();
                                    // addNodeC1Button->depush();
                                });
    posY += buttonHeight + spacingY;
    addNodeC2Button = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 150, 250),
        "C2",
        "Arial",
        true);

    addNodeC2Button->setOnClick([this, &state]()
                                {
            if(state.getEditorMode() == EditorMode::AddingC2Node){
                    state.setEditorMode(EditorMode::Idle);
                
            }
            else{
            state.setEditorMode(EditorMode::AddingC2Node);
            addNodeC3Button->depush();
            addNodeC1Button->depush();
            addLinkButton->depush();
            cutNodeButton->depush();
            cutLinkButton->depush();
            moveNodeButton->depush();
            link.reset();
            } });

    posY += buttonHeight + spacingY;
    addNodeC3Button = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 150, 250),
        "C3",
        "Arial",
        true);

    addNodeC3Button->setOnClick([this, &state]()
                                {
            if(state.getEditorMode() == EditorMode::AddingC3Node){
                    state.setEditorMode(EditorMode::Idle);
            }
            else{
            state.setEditorMode(EditorMode::AddingC3Node);
            addNodeC2Button->depush();
            addNodeC1Button->depush();
            addLinkButton->depush();
            cutNodeButton->depush();
            cutLinkButton->depush();
            moveNodeButton->depush();
            link.reset();
            } });

    posY += buttonHeight + spacingY;

    addLinkButton = std::make_unique<Button>(
        startX,
        posY,
        130.f,
        50.f,
        sf::Color::Magenta,
        "Add Link",
        "Arial",
        true);
    addLinkButton->setOnClick([this, &state]()
                             {
            if(state.getEditorMode() == EditorMode::AddingLink){
                    state.setEditorMode(EditorMode::Idle);
            }
            else{
            state.setEditorMode(EditorMode::AddingLink);
            addNodeC2Button->depush();
            addNodeC1Button->depush();
            addNodeC3Button->depush();
            cutLinkButton->depush();
            cutNodeButton->depush();
            moveNodeButton->depush();
            link.reset();
            } });

    posY += 50.f + spacingY;

    cutNodeButton = std::make_unique<Button>(
        startX,
        posY,
        130.f,
        50.f,
        sf::Color::Red,
        "Cut Node",
        "Arial",
        true);
    cutNodeButton->setOnClick([this, &state]()
                             {
            if(state.getEditorMode() == EditorMode::CuttingNode){
                    state.setEditorMode(EditorMode::Idle);
            }
            else{
            state.setEditorMode(EditorMode::CuttingNode);
            addNodeC2Button->depush();
            addNodeC1Button->depush();
            addNodeC3Button->depush();
            addLinkButton->depush();
            cutLinkButton->depush();
            moveNodeButton->depush();

            link.reset();

            } }
        );

    posY += 50.f + spacingY;

    cutLinkButton = std::make_unique<Button>(
        startX,
        posY,
        130.f,
        50.f,
        sf::Color::Red,
        "Cut Link",
        "Arial",
        true);
    cutLinkButton->setOnClick([this, &state]()
                             {
            if(state.getEditorMode() == EditorMode::CuttingLink){
                    state.setEditorMode(EditorMode::Idle);
            }
            else{
            state.setEditorMode(EditorMode::CuttingLink);
            addNodeC2Button->depush();
            addNodeC1Button->depush();
            addNodeC3Button->depush();
            addLinkButton->depush();
            cutNodeButton->depush();
            moveNodeButton->depush();
            link.reset();

            } }
        );

    posY += 50.f + spacingY;

    moveNodeButton = std::make_unique<Button>(
        startX,
        posY,
        130.f,
        50.f,
        sf::Color::Red,
        "Move Node",
        "Arial",
        true);
    moveNodeButton->setOnClick([this, &state]()
                             {
            if(state.getEditorMode() == EditorMode::MovingNode){
                    state.setEditorMode(EditorMode::Idle);
            }
            else{
            state.setEditorMode(EditorMode::MovingNode);
            addNodeC2Button->depush();
            addNodeC1Button->depush();
            addNodeC3Button->depush();
            addLinkButton->depush();
            cutNodeButton->depush();
            cutLinkButton->depush();
            link.reset();

            } }
        );
}

void TopologyEditorManager::handleInput(const InputManager &input)
{
    sf::Vector2f mouseWorld = input.getMouseWorldPosition();

    std::ostringstream ss;
    ss << "X: " << static_cast<int>(mouseWorld.x) << " Y: " << static_cast<int>(mouseWorld.y);
    coordText->setString(ss.str());

    modeDropdown.update(input);

    // needs to be before the button updates
    // needs to do C1
    if (topologyBounds.getGlobalBounds().contains(input.getMouseUIScreenPosition()))
    {
        if(state.getEditorMode() == EditorMode::MovingNode && selectedNodeId.has_value())
        {
            auto it = visualNodes.find(*selectedNodeId);
            if (it != visualNodes.end())
            {
                auto &device = it->second;
                device->changePosition(mouseWorld);
                // state.changeNodePosition(device->nodeId, mouseWorld);
            }
        }

        else if (state.getEditorMode() == EditorMode::AddingC2Node && input.isMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C2;
            // for the save logic
            state.addNode(nodeCounter, cls, mouseWorld);
            // for the visualisation
            addVisualNode(nodeCounter, cls, mouseWorld);
            ++nodeCounter;
        }

        else if (state.getEditorMode() == EditorMode::AddingC3Node && input.isMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C3;
            // for the save logic
            state.addNode(nodeCounter, cls, mouseWorld);
            // for the visualisation
            addVisualNode(nodeCounter, cls, mouseWorld);
            ++nodeCounter;
        }

 

        bool CuttingNode=(state.getEditorMode() == EditorMode::CuttingNode && input.isMouseJustPressed());
        bool AddingLink=(state.getEditorMode() == EditorMode::AddingLink && input.isMouseJustPressed());
        bool CuttingLink=(state.getEditorMode() == EditorMode::CuttingLink && input.isMouseJustPressed());
        bool MovingNode=(state.getEditorMode() == EditorMode::MovingNode && input.isMouseJustPressed());
        //manual iterator to avoid invalidating the iterator when erasing
        for (auto it = visualNodes.begin(); it != visualNodes.end();)
        {
            auto &device = it->second;
            device->update(input);

            if (CuttingNode && device->getIsHovered())
            {
                // Remove the routing inward and outward
                std::cout << "********Removing node " << device->nodeId << "********.\n";
                removeRouting(device->nodeId, std::nullopt);
                removeRouting(std::nullopt, device->nodeId);
                std::cout << "********After Removing node " << device->nodeId << "********.\n";

                state.removeNode(device->nodeId);
                it = visualNodes.erase(it); // ✅ erase returns next valid iterator
            }
            else
            {
                if (AddingLink && device->getIsHovered())
                {
                    link.add(device->nodeId);
                    if (link.isComplete())
                    {
                        // Add the routing
                        addRouting(link.nodes[0], link.nodes[1]);
                        // Reset the link
                        link.reset();
                    }
                }
                else if(CuttingLink && device->getIsHovered())
                {
                    // Check if the link is complete
                    link.add(device->nodeId);
                    if (link.isComplete())
                    {
                        // Remove the routing
                        removeRouting(link.nodes[0], link.nodes[1]);
                        // Reset the link
                        link.reset();
                    }
                }
                else if (MovingNode && device->getIsHovered())
                {
                    // Move the node
                    // state.changeNodePosition(device->nodeId, mouseWorld);
                    if(selectedNodeId.has_value())
                    {
                        selectedNodeId = std::nullopt;
                    }
                    else{
                        selectedNodeId = device->nodeId;

                    }
                }

                ++it; // Only increment if not erased
            }
        }
    }

    if (saveButton)
        saveButton->update(input);
    if (addNodeC1Button)
        addNodeC1Button->update(input);
    if (addNodeC2Button)
        addNodeC2Button->update(input);
    if (addNodeC3Button)
        addNodeC3Button->update(input);

    if (addLinkButton)
        addLinkButton->update(input);
    if (cutNodeButton)
        cutNodeButton->update(input);
    if(cutLinkButton)
        cutLinkButton->update(input);
    if (moveNodeButton)
        moveNodeButton->update(input);


}

bool TopologyEditorManager::addRouting(int id1, int id2)
{
    if(id1 == id2)
    {
        std::cout << "********Cannot connect a device to itself.********\n";
        return false;
    }
    // ID1 ---> ID2 , path towards ID2
    if (visualNodes.count(id1) && visualNodes.count(id2))
    {
        // Check if the devices are already connected
        if (routings[id1].count(id2) > 0)
        {
            std::cout << "********Devices are already connected.********\n";
            return false;
        }
        
     else   // Add the routing
    {
        routings[id1].insert(id2); // Only store id2 in id1's adjacency list
        return true;
    } }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
        return false;
    }
}

bool TopologyEditorManager::removeRouting(std::optional<int> id1, std::optional<int> id2)
{
    // Invalid if neither are provided
    if (!id1 && !id2) {
        std::cerr << "Error: Must provide at least one ID.\n";
        return false;
    }

    if (id1 && id2) {
        if(*id1 == *id2)
        {
            std::cout << "********Cannot remove a link to itself.********\n";
            return false;
        }

        // Case 1: remove specific link from id1 to id2
        if (!visualNodes.count(*id1) || !visualNodes.count(*id2)) {
            std::cout << "********One or both devices do not exist.********\n";
            return false;
        }

        auto it = routings.find(*id1);
        if (it != routings.end() && it->second.erase(*id2) > 0) {
            return true;
        } else {
            std::cout << "********Devices are not connected.********\n";
            return false;
        }
    }

    // One of the two is set
    int targetId = id1 ? *id1 : *id2;

    if (!visualNodes.count(targetId)) {
        std::cout << "********Device " << targetId << " does not exist.********\n";
        return false;
    }

    bool removedAny = false;

    // Remove all outbound links
    if (routings.erase(targetId) > 0) {
        removedAny = true;
    }

    // Remove all inbound links
    for (auto& [from, targets] : routings) {
        if (targets.erase(targetId) > 0) {
            removedAny = true;
        }
    }

    if (!removedAny) {
        std::cout << "********No routings found involving device " << targetId << ".********\n";
    }

    return removedAny;
}


void  TopologyEditorManager::drawRootings(sf::RenderWindow &window)
{
    //Todo: you can probably optimize this function, too much nested for loops
    
    //for each Device...
    for (const auto &[deviceId, connectedDevicesId] : routings)
    {
        //for each connected device to the first device...
        for (const auto &connectedDeviceId : connectedDevicesId)
        {
            // get start position and end position of these two devices
            sf::Vector2f start;
            sf::Vector2f end;
            bool foundPos = false;

            auto it = visualNodes.find(connectedDeviceId);
            if (it != visualNodes.end()) {
                // Access the value
                start =  it->second->getPosition();
                // center the start position
                    start.x += config::radiusIcon;
                    start.y += config::radiusIcon;
                
                auto it2 = visualNodes.find(deviceId);
                if (it2 != visualNodes.end()) {
                    // Access the value
                    end =  it2->second->getPosition();
                    // center the end position
                    end.x += config::radiusIcon;
                    end.y += config::radiusIcon;
                    foundPos = true;
                }
                else {
                    throw (std::runtime_error("Device2 not found in Rooting Drawing"));
                }
            } else {
                throw (std::runtime_error("Device1 not found in Rooting Drawing"));
            }

            if (foundPos)
            {
                drawArrowWithHeads(window, start, end, 35.f);
            }
        }
    }
}


void TopologyEditorManager::update(float deltaTime)
{
}

void TopologyEditorManager::setSelectedNode(std::optional<int> nodeId)
{
}

std::optional<int> TopologyEditorManager::getSelectedNode() const
{
    return std::optional<int>();
}

void TopologyEditorManager::addVisualNode(int id, DeviceClass cls, sf::Vector2f pos)
{
    auto battery = 100.0; // dummy default
    int dClass = 0;

    if (cls == DeviceClass::C1)
    {
        dClass = 1;
    }
    else if (cls == DeviceClass::C2)
    {
        dClass = 2;
    }
    else if (cls == DeviceClass::C3)
    {
        dClass = 3;
    }

    visualNodes[id] = std::make_unique<Device>(id, dClass, std::make_pair((int)pos.x, (int)pos.y), battery);
}

std::optional<int> TopologyEditorManager::getNodeAtPosition(sf::Vector2f position)
{

    return std::optional<int>();
}

void TopologyEditorManager::draw(sf::RenderWindow &window, sf::View &editorView)
{
    sf::View defaultView = window.getDefaultView();
    window.setView(defaultView);
    window.draw(*coordText);

    // draw the topology bounds rectangle adapting to the screen size.
    // Recalculate size based on window dimensions
    sf::Vector2f windowSize = defaultView.getSize();
    sf::Vector2f viewCenter = defaultView.getCenter();
    sf::Vector2f rectSize(
        0.8f * windowSize.x,
        0.8f * windowSize.y);

    topologyBounds.setSize(rectSize);

    // Center it
    topologyBounds.setPosition({viewCenter.x - rectSize.x / 2.f,
                                viewCenter.y - rectSize.y / 2.f});

    window.draw(topologyBounds);

    modeDropdown.draw(window);
    if (saveButton)
        saveButton->draw(window);
    if (addNodeC1Button)
        addNodeC1Button->draw(window);
    if (addNodeC2Button)
        addNodeC2Button->draw(window);
    if (addNodeC3Button)
        addNodeC3Button->draw(window);
    if (addLinkButton)
        addLinkButton->draw(window);
    if (cutNodeButton)
        cutNodeButton->draw(window);
    if (cutLinkButton)
        cutLinkButton->draw(window);
    if (moveNodeButton)
        moveNodeButton->draw(window);


    window.setView(editorView);
    sf::RectangleShape worldRect = convertRectangleToTopologyView(window, editorView, topologyBounds);

    for (auto &[_, device] : visualNodes)
    {
        if (isBoundsFullyInsideRect(device->getPosition(), device->getSize(), worldRect.getGlobalBounds()))
        {
            device->draw(window);
        }
    }

    drawRootings(window);
}

sf::RectangleShape TopologyEditorManager::convertRectangleToTopologyView(
    const sf::RenderWindow &window,
    const sf::View &topologyView,
    const sf::RectangleShape &screenRect)
{
    // Extract position and size in default view space
    sf::Vector2f topLeftDefault = screenRect.getPosition();
    sf::Vector2f sizeDefault = screenRect.getSize();

    // Convert top-left and bottom-right corners to topology view space
    sf::Vector2f topLeft = window.mapPixelToCoords(
        sf::Vector2i(static_cast<int>(topLeftDefault.x), static_cast<int>(topLeftDefault.y)),
        topologyView);

    sf::Vector2f bottomRight = window.mapPixelToCoords(
        sf::Vector2i(static_cast<int>(topLeftDefault.x + sizeDefault.x),
                     static_cast<int>(topLeftDefault.y + sizeDefault.y)),
        topologyView);

    // Create and return a new rectangle in topology view space
    sf::RectangleShape worldRect;
    worldRect.setPosition(topLeft);
    worldRect.setSize(bottomRight - topLeft);
    return worldRect;
}

bool TopologyEditorManager::isBoundsFullyInsideRect(
    const sf::Vector2f &position, // top-left corner
    const sf::Vector2f &size,
    const sf::FloatRect &rect)
{
    sf::Vector2f topLeft = position;
    sf::Vector2f bottomRight = position + size;

    return rect.contains(topLeft) && rect.contains(bottomRight);
}
