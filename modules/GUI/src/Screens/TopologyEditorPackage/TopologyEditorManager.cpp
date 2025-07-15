#include "TopologyEditorManager.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"
#include "TopologyConfigIO.hpp"

#include "../../UI/UIFactory/UIFactory.hpp"

TopologyEditorManager::TopologyEditorManager(TopologyEditorState &state, tgui::Gui &gui)
    : selectedNodeId(std::nullopt), state(state), gui(gui)
{
}

void TopologyEditorManager::setupUI(tgui::Gui &gui, sf::View &editorView)
{

    coordLabel = UIFactory::createLabel("X: -, Y: -");
    coordLabel->setPosition({"40%", "2%"});
    gui.add(coordLabel);

    // Create a ComboBox for TopologyMode selection
    auto comboBox = UIFactory::createEnumComboBox({"15%", "5%"});
    comboBox->setPosition({"9%", "2%"});

    // Add each enum value by name
    for (auto mode : magic_enum::enum_values<TopologyMode>())
    {
        std::string label = std::string(magic_enum::enum_name(mode));
        comboBox->addItem(label, label);
    }

    comboBox->onItemSelect([this, comboBox]
                           {
        auto selected = comboBox->getSelectedItem();
         if (auto modeOpt = magic_enum::enum_cast<TopologyMode>(selected.toStdString())) {
            state.setTopologyMode(*modeOpt);
             std::cout << "Selected mode: " << selected.toStdString() << std::endl;
         } });
    gui.add(comboBox);

    auto saveButton = UIFactory::createButton("Save",
                                              [this]
                                              {
                                                  //   std::string filename = fileNameBox->getText().toStdString() + ".simcfg";
                                                  //   TopologyConfigIO::write(filename, state.getNodes(), state.getRoutings(), state.getTopologyMode());
                                              });
    saveButton->setSize({"7%", "4%"});
    saveButton->setPosition({"1%", "7%"});
    gui.add(saveButton);

    std::vector<std::pair<std::string, EditorMode>> toolData = {
        {"C1", EditorMode::AddingC1Node},
        {"C2", EditorMode::AddingC2Node},
        {"C3", EditorMode::AddingC3Node},
        {"Add Link", EditorMode::AddingLink},
        {"Cut Node", EditorMode::CuttingNode},
        {"Cut Link", EditorMode::CuttingLink},
        {"Move Node", EditorMode::MovingNode}};

    const float basePercent = 20.f;
    const float percentStep = 6.f;
    const tgui::String xPos = "1%";
    const tgui::String sizeX = "7%";
    const tgui::String sizeY = "4.5%";

    for (std::size_t i = 0; i < toolData.size(); ++i)
    {
        const auto &[label, mode] = toolData[i];

        auto button = UIFactory::createToggleButton(label);
        button->setSize({sizeX, sizeY});
        button->setPosition({xPos, tgui::String(std::to_string(basePercent + i * percentStep) + "%")});

        button->onClick([this, mode, button]
                        {
        if (state.getEditorMode() == mode) {
            // Toggle off
            state.setEditorMode(EditorMode::Idle);
            button->setDown(false);
        } else {
            // Toggle on and unpress others
            for (auto& [otherMode, btn] : toolButtons)
                btn->setDown(false);

            button->setDown(true);
            state.setEditorMode(mode);
            state.resetLink();
        } });

        gui.add(button);
        toolButtons[mode] = button;
    }

    // TODO: button to display the range aka broadcast

    // for pure SFML content (nodes, rootings...)
    canvas = tgui::CanvasSFML::create();
    canvas->setSize({"85%", "85%"});
    canvas->setPosition({"13%", "13%"});
    canvas->clear(tgui::Color(30, 30, 30));
    gui.add(canvas);

    //we need to fit the view to the canvas dimensions.
    editorView.setCenter({canvas->getAbsolutePosition().x+canvas->getSize().x/2,canvas->getAbsolutePosition().y+canvas->getSize().y/2} );
    editorView.setSize({canvas->getSize().x,canvas->getSize().y});
    canvas->setView(editorView);
}

void TopologyEditorManager::handleInput(InputManager &input)
{

    sf::Vector2i mouseWorld = input.getMouseScreenPosition();
    tgui::Vector2f guiMousePos = gui.mapPixelToCoords(mouseWorld);


    tgui::Vector2f canvasPos = canvas->getAbsolutePosition();
    tgui::Vector2f canvasSize = canvas->getSize();

    std::unordered_map<int, std::unique_ptr<Device>> &nodes = state.getNodes();

    //within the canvas
    if (canvasPos.x <= guiMousePos.x &&
        canvasPos.y <= guiMousePos.y &&
        guiMousePos.x <= canvasPos.x + canvasSize.x &&
        guiMousePos.y <= canvasPos.y + canvasSize.y)
    {
        // Mouse clicked inside canvas
        // Convert to canvas-local coords
        sf::Vector2f localCanvasPos = guiMousePos - canvas->getAbsolutePosition();

        // Convert to world coordinates if using view
        sf::Vector2f worldPos = canvas->mapPixelToCoords(tgui::Vector2f(localCanvasPos) );
        std::ostringstream ss;
            ss << "X: " << static_cast<int>(localCanvasPos.x) << " Y: " << static_cast<int>(localCanvasPos.y);
            coordLabel->setText(ss.str());
    
        if (state.getEditorMode() == EditorMode::MovingNode && selectedNodeId.has_value())
        {
            auto it = nodes.find(*selectedNodeId);
            if (it != nodes.end())
            {
                auto &device = it->second;
                device->changePosition(sf::Vector2f(mouseWorld));
                // state.changeNodePosition(device->nodeId, mouseWorld);
            }
        }

        else if (state.getEditorMode() == EditorMode::AddingC1Node && input.isLeftMouseJustPressed())
        {
            // TODO
        }
        else if (state.getEditorMode() == EditorMode::AddingC2Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C2;

            state.addNode(cls, sf::Vector2f(mouseWorld));
        }

        else if (state.getEditorMode() == EditorMode::AddingC3Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C3;
            // for the save logic
            state.addNode(cls, sf::Vector2f(mouseWorld));
        }
        bool CuttingNode = (state.getEditorMode() == EditorMode::CuttingNode && input.isLeftMouseJustPressed());
        bool AddingLink = (state.getEditorMode() == EditorMode::AddingLink && input.isLeftMouseJustPressed());
        bool CuttingLink = (state.getEditorMode() == EditorMode::CuttingLink && input.isLeftMouseJustPressed());
        bool MovingNode = (state.getEditorMode() == EditorMode::MovingNode && input.isLeftMouseJustPressed());
        bool NodeBroadcast = (input.isRightMouseJustPressed());
        // manual iterator to avoid invalidating the iterator when erasing
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

            // for operations that do not involve suppressing the node (otherwise it messes up with the iterator)
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
                else if (CuttingLink && device->getIsHovered())
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
                    if (selectedNodeId.has_value())
                    {
                        selectedNodeId = std::nullopt;
                    }
                    else
                    {
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
    // broadcast animations


    

  }

void TopologyEditorManager::startBroadcast(const sf::Vector2f &startPosition, float duration)
{
    auto animation = std::make_unique<BroadcastAnimation>(startPosition, duration);
    broadcastAnimations.push_back(std::move(animation));
}

void TopologyEditorManager::drawRootings(tgui::CanvasSFML::Ptr canvas)
{

    // for each Device...
    const auto &routings = state.getRoutings();
    const auto &nodes = state.getNodes();
    for (const auto &[deviceId, connectedDevicesId] : routings)
    {
        for (const auto &connectedDeviceId : connectedDevicesId)
        {
            sf::Vector2f start, end;
            if (nodes.count(deviceId) && nodes.count(connectedDeviceId))
            {
                start = nodes.at(deviceId)->getCenteredPosition();        // source
                end = nodes.at(connectedDeviceId)->getCenteredPosition(); // destination
                drawArrowWithHeads(canvas, start, end, 35.f);
            }
            else
            {
                throw std::runtime_error("Device not found in Rooting Drawing");
            }
        }
    }
}

// delta time is useless, get rid of it todo
void TopologyEditorManager::update(float deltaTime)
{
    for (auto &animation : broadcastAnimations)
        animation->update();
    broadcastAnimations.erase(std::remove_if(broadcastAnimations.begin(), broadcastAnimations.end(),
                                             [](const std::unique_ptr<BroadcastAnimation> &animation)
                                             { return animation->isFinished(); }),
                              broadcastAnimations.end());
}


//THESE two are not needed
void TopologyEditorManager::setSelectedNode(std::optional<int> nodeId)
{

    // TODO: still needed?
}

std::optional<int> TopologyEditorManager::getSelectedNode() const
{
    return selectedNodeId;
}

void TopologyEditorManager::draw(sf::RenderWindow &window, sf::View &editorView)
{
    //canvas stores a copy of the view, so we need to set it everytime we draw
    canvas->setView(editorView);
    canvas->clear(tgui::Color(30, 30, 30));

    // sf::View defaultView = window.getDefaultView();
    // window.setView(defaultView);

    // draw the topology bounds rectangle adapting to the screen size.
    // Recalculate size based on window dimensions
    // sf::Vector2f windowSize = defaultView.getSize();
    // sf::Vector2f viewCenter = defaultView.getCenter();
    // sf::Vector2f rectSize(
    //     0.8f * windowSize.x,
    //     0.8f * windowSize.y);

    for (auto &[_, device] : state.getNodes())
    {

        device->draw(canvas);
    }

    drawRootings(canvas);
    for (auto &animation : broadcastAnimations)
        animation->draw(window);
    canvas->display();
}


//todo: useless?
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
