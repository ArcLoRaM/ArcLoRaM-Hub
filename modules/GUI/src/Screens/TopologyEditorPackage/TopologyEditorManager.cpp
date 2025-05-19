#include "TopologyEditorManager.hpp"
#include <iostream>

TopologyEditorManager::TopologyEditorManager(TopologyEditorState &state)
    : nodeCounter(0), selectedNodeId(std::nullopt),
      modeDropdown(180.f, 30.f), state(state)
{

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
    posY += buttonHeight + spacingY;

    saveButton = std::make_unique<Button>(
        startX,
        posY,
        150.f,
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
            } });

    posY += buttonHeight + spacingY;

    addLinkButton = std::make_unique<Button>(
        startX,
        posY,
        150.f,
        50.f,
        sf::Color::Magenta,
        "Add Link",
        "Arial",
        true);
    addLinkButton->setOnClick([this]() {

    });
}

void TopologyEditorManager::handleInput(const InputManager &input)
{
    sf::Vector2f mouseWorld = input.getMouseWorldPosition();
    modeDropdown.update(input);

    // needs to be before the button updates
    // needs to do C1
    if (topologyBounds.getGlobalBounds().contains(input.getMouseUIScreenPosition()))
    {

        if (state.getEditorMode() == EditorMode::AddingC2Node && input.isMouseJustPressed())
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

    // else if (mode == EditorMode::AddingLink && input.isMouseJustPressed()) {
    //     auto target = getNodeAtPosition(mouseWorld);
    //     if (selectedNodeId && target && *selectedNodeId != *target) {
    //         state.setNextHop(*selectedNodeId, *target, /*calculate hopCount*/ 1);
    //         selectedNodeId = std::nullopt;
    //     } else {
    //         selectedNodeId = target;
    //     }
    // }

    for (auto &[_, device] : visualNodes)
    {
        device->update(input);
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



    window.setView(editorView);
    for (auto &[_, device] : visualNodes)
    {
        
        device->draw(window);
    }

    // Draw routing too...
}