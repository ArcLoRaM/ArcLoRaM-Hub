#include "TopologyEditorManager.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"
#include "TopologyConfigIO.hpp"


TopologyEditorManager::TopologyEditorManager(TopologyEditorState &state)
    :  selectedNodeId(std::nullopt),
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
    saveButton->setOnClick([this,&state]()
                           {
                            TopologyConfigIO::write("topology_config.txt", state.getNodes(), state.getRoutings(), state.getTopologyMode());
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
            state.resetLink();
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
            state.resetLink();
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
            state.resetLink();
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
            state.resetLink();

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
            state.resetLink();

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
            state.resetLink();

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

    std::unordered_map<int, std::unique_ptr<Device>>& nodes = state.getNodes();

    if (topologyBounds.getGlobalBounds().contains(input.getMouseUIScreenPosition()))
    {
        if(state.getEditorMode() == EditorMode::MovingNode && selectedNodeId.has_value())
        {
            auto it = nodes.find(*selectedNodeId);
            if (it != nodes.end())
            {
                auto &device = it->second;
                device->changePosition(mouseWorld);
                // state.changeNodePosition(device->nodeId, mouseWorld);
            }
        }

        else if (state.getEditorMode() == EditorMode::AddingC1Node && input.isLeftMouseJustPressed())
        {
            //TODO
            
        }
        else if (state.getEditorMode() == EditorMode::AddingC2Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C2;
      
            state.addNode( cls, mouseWorld);
            
        }

        else if (state.getEditorMode() == EditorMode::AddingC3Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C3;
            // for the save logic
            state.addNode( cls, mouseWorld);
 
        }

 

        bool CuttingNode=(state.getEditorMode() == EditorMode::CuttingNode && input.isLeftMouseJustPressed());
        bool AddingLink=(state.getEditorMode() == EditorMode::AddingLink && input.isLeftMouseJustPressed());
        bool CuttingLink=(state.getEditorMode() == EditorMode::CuttingLink && input.isLeftMouseJustPressed());
        bool MovingNode=(state.getEditorMode() == EditorMode::MovingNode && input.isLeftMouseJustPressed());
        bool NodeBroadcast=(input.isRightMouseJustPressed());
        //manual iterator to avoid invalidating the iterator when erasing
        for (auto it = nodes.begin(); it != nodes.end();)
        {
            auto &device = it->second;
            device->update(input);
                int nodeId = device->getNodeId();

            if (CuttingNode && device->getIsHovered())
            {
                // Remove the routing inward and outward
                std::cout << "********Removing node " << nodeId << "********.\n";
                state.removeRouting(nodeId, std::nullopt);
                state.removeRouting(std::nullopt, nodeId);
                it = state.removeNode(it);
                std::cout << "********After Removing node " << nodeId << "********.\n";
            }

            //for operations that do not involve suppressing the node (otherwise it messes up with the iterator)
            else
            {
                if (AddingLink && device->getIsHovered())
                {
                                        // The pending Link is used for creation and deletion, is it cleared when we change mode

                    state.addLink(nodeId);
                    if (state.isLinkComplete())
                    {
                        // Add the routing
                        state.addRouting();
                        // Reset the link
                        state.resetLink();
                    }
                }
                else if(CuttingLink && device->getIsHovered())
                {
                    // The pending Link is used for creation and deletion, is it cleared when we change mode
                    state.addLink(device->getNodeId());
                    if (state.isLinkComplete())
                    {
                        // Remove the routing
                        state.removeSpecificRouting();
                        // Reset the link
                        state.resetLink();
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
                        selectedNodeId = device->getNodeId();

                    }
                }
                else if (NodeBroadcast && device->getIsHovered())
                {
                    // Start the broadcast animation
                    startBroadcast(device->getCenteredPosition(), config::broadcastDuration);
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


    //broadcast animations




}





void TopologyEditorManager::startBroadcast(const sf::Vector2f &startPosition, float duration)
{
    auto animation = std::make_unique<BroadcastAnimation>(startPosition, duration);
    broadcastAnimations.push_back(std::move(animation));
}



void  TopologyEditorManager::drawRootings(sf::RenderWindow &window)
{
    

    //for each Device...
    const auto& routings = state.getRoutings();
    const auto& nodes = state.getNodes();
    for (const auto &[deviceId, connectedDevicesId] : routings) {
        for (const auto &connectedDeviceId : connectedDevicesId) {
            sf::Vector2f start, end;
            if (nodes.count(deviceId) && nodes.count(connectedDeviceId)) {
                start = nodes.at(deviceId)->getCenteredPosition();         // source
                end   = nodes.at(connectedDeviceId)->getCenteredPosition(); // destination
                drawArrowWithHeads(window, start, end, 35.f);
            } else {
                throw std::runtime_error("Device not found in Rooting Drawing");
            }
        }
    }
}


void TopologyEditorManager::update(float deltaTime)
{
for (auto &animation : broadcastAnimations)
        animation->update();
    broadcastAnimations.erase(std::remove_if(broadcastAnimations.begin(), broadcastAnimations.end(),
                        [](const std::unique_ptr<BroadcastAnimation> &animation)
                        { return animation->isFinished(); }),
        broadcastAnimations.end());
}

void TopologyEditorManager::setSelectedNode(std::optional<int> nodeId)
{
}

std::optional<int> TopologyEditorManager::getSelectedNode() const
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

    for (auto &[_, device] : state.getNodes())
    {
        if (isBoundsFullyInsideRect(device->getPosition(), device->getSize(), worldRect.getGlobalBounds()))
        {
            device->draw(window);
        }
    }

    drawRootings(window);
    for (auto &animation : broadcastAnimations)
        animation->draw(window);
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
