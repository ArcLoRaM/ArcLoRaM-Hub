#include "TopologyEditorManager.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"
#include "TopologyConfigIO.hpp"
#include <filesystem>  // C++17
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
    modeDropdown = UIFactory::createEnumComboBox({"15%", "5%"});
    modeDropdown->setPosition({"9%", "2%"});

    // Add each enum value by name
    for (auto mode : magic_enum::enum_values<TopologyMode>())
    {
        std::string label = std::string(magic_enum::enum_name(mode));
        modeDropdown->addItem(label, label);
    }

    modeDropdown->onItemSelect([this]
                           {
        resetToggleButtons();
        auto selected = modeDropdown->getSelectedItem();
         if (auto modeOpt = magic_enum::enum_cast<TopologyMode>(selected.toStdString())) {
            state.setTopologyMode(*modeOpt);
             std::cout << "Selected mode: " << selected.toStdString() << std::endl;
         } });

    modeDropdown->onFocus([this]
                           {
        resetToggleButtons();
                           });
    gui.add(modeDropdown);

    auto saveButton = UIFactory::createButton("Save",[this, &gui]{
        resetToggleButtons();
        if(state.getNodes().empty()) {
            auto errorBox = UIFactory::createMessageBox("Error", "No nodes to save.");
            errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                if(button == "OK" ){
                    msgBox->getParent()->remove(msgBox->shared_from_this()); 
                } 
            });
            gui.add(errorBox);
            return;
        }

        if((modeDropdown->getSelectedItem()).empty()) {
            auto errorBox = UIFactory::createMessageBox("Error", "Please select a topology mode.");
            errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                if(button == "OK" ){
                    msgBox->getParent()->remove(msgBox->shared_from_this()); 
                } 
            });
            gui.add(errorBox);
            return;
        }


        //TODO: put more check control on the topology (tree type for RRC Uplink, at least one C3....), no circle rooting...
        auto targetDir = std::filesystem::path("output/topologies");
        
        // Auto-create directory if it doesn't exist
        if (!std::filesystem::exists(targetDir))
            std::filesystem::create_directories(targetDir);

        //saveFileDialog
        auto dialog = tgui::FileDialog::create("Save Topology", "Save");
        dialog->setFileMustExist(false);
        dialog->setPath(targetDir.string());  // Set default dir
        dialog->setFilename("topology.simcfg");
        // dialog->setFileTypeFilters( {{ "Simulation Config (*.simcfg)", "*.simcfg" }});
        dialog->setTitle("Save Topology Configuration");
        dialog->setSize({"50%", "50%"});
        dialog->setPosition({"25%", "25%"});
        dialog->setFileTypeFilters({ {"Simulation File", {"*.simcfg"}} }, 0);
      //convert "2%" to pixels based on window height
        dialog->onFileSelect([this,&gui](const tgui::String& filePath) {
            if(!TopologyConfigIO::write(filePath.toStdString(), state.getNodes(), state.getRoutings(), state.getTopologyMode())){
                auto errorBox = UIFactory::createMessageBox("Incorrect Topology", "An Invalid file was created");
                errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                if(button == "OK" ){
                    msgBox->getParent()->remove(msgBox->shared_from_this()); 
                } 
            });
            gui.add(errorBox);
            return;
            };
        });

        gui.add(dialog);
        //after add
        unsigned int textSize = 0.015f * gui.getView().getRect().height;  // e.g. 2.5% of height
        dialog->setTextSize(textSize);
        dialog->getRenderer()->setTextSize(textSize);

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
const tgui::String sizeX = "9%";
const tgui::String sizeY = "5.5%";

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

auto rangeButton = UIFactory::createButton("Range",[this](){
    for(auto& [_,node] : state.getNodes()) {
        startBroadcast(node->getCenteredPosition(), 2.f); 
    }
});
rangeButton->setSize({"7%", "4%"});
rangeButton->setPosition({"1%", "70%"});

gui.add(rangeButton);

// for pure SFML content (nodes, rootings...)
canvas = tgui::CanvasSFML::create();
canvas->setSize({"85%", "85%"});
canvas->setPosition({"13%", "13%"});
canvas->clear(tgui::Color(30, 30, 30));
gui.add(canvas);

// we need to fit the view to the canvas dimensions.
editorView.setCenter({canvas->getAbsolutePosition().x + canvas->getSize().x / 2, canvas->getAbsolutePosition().y + canvas->getSize().y / 2});
editorView.setSize({canvas->getSize().x, canvas->getSize().y});
canvas->setView(editorView);

}

void TopologyEditorManager::handleInput(InputManager &input)
{

    sf::Vector2i mouseWinPos  = input.getMouseScreenPosition();

    tgui::Vector2f canvasPos = canvas->getPosition();
    tgui::Vector2f canvasSize = canvas->getSize();

// Step 3: Convert to pixel position relative to canvas
    sf::Vector2i canvasPixel = sf::Vector2i(
    static_cast<int>(mouseWinPos.x - canvasPos.x),
    static_cast<int>(mouseWinPos.y - canvasPos.y)
    );


    std::unordered_map<int, std::unique_ptr<Device>> &nodes = state.getNodes();

        if (canvasPixel.x < 0 || canvasPixel.y < 0 ||
            canvasPixel.x >= static_cast<int>(canvasSize.x) ||
            canvasPixel.y >= static_cast<int>(canvasSize.y))
        {
            return; // Outside canvas, skip
        }


        // MMouse inside the canvas

        sf::Vector2f localCanvasPos = canvas->mapPixelToCoords(tgui::Vector2f(tgui::Vector2i(canvasPixel))); // Uses canvas's active view


        
        std::ostringstream ss;
        ss << "X: " << static_cast<int>(localCanvasPos.x) << " Y: " << static_cast<int>(localCanvasPos.y);
        coordLabel->setText(ss.str());

        if (state.getEditorMode() == EditorMode::MovingNode && selectedNodeId.has_value())
        {
            auto it = nodes.find(*selectedNodeId);
            if (it != nodes.end())
            {
                auto &device = it->second;
                device->changePosition(sf::Vector2f(localCanvasPos));
            }
        }

        else if (state.getEditorMode() == EditorMode::AddingC1Node && input.isLeftMouseJustPressed())
        {
            // TODO
        }
        else if (state.getEditorMode() == EditorMode::AddingC2Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C2;

            state.addNode(cls, localCanvasPos );
        }

        else if (state.getEditorMode() == EditorMode::AddingC3Node && input.isLeftMouseJustPressed())
        {
            DeviceClass cls = DeviceClass::C3;
            // for the save logic
            state.addNode(cls, localCanvasPos );
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
            device->update(input,gui,canvas);
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




void TopologyEditorManager::resetToggleButtons()
{
    for(auto &[_, button] : toolButtons)
    {
        button->setDown(false);
    }
    state.setEditorMode(EditorMode::Idle);
    state.resetLink();
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

// THESE two are not needed
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
    // canvas stores a copy of the view, so we need to set it everytime we draw
    canvas->setView(editorView);
    canvas->clear(tgui::Color(30, 30, 30));

    for (auto &[_, device] : state.getNodes())
    {

        device->draw(canvas);
    }

    drawRootings(canvas);
    for (auto &animation : broadcastAnimations)
        animation->draw(canvas);
    canvas->display();
}

