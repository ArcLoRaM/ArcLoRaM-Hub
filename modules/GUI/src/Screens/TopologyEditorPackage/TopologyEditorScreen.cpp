#include "TopologyEditorScreen.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"
#include "TopologyConfigIO.hpp"
#include "../../UI/UIFactory/UIFactory.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../Shared/Libraries/magic_enum.hpp"

TopologyEditorScreen::TopologyEditorScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
    : Screen(gui),editorView(sf::FloatRect({0, 0}, {0,0}))
    , editorManager(editorState,gui)
{
    setupUI(actions);
    editorManager.setupUI(editorView);
}

TopologyEditorScreen::TopologyEditorScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui,const std::string& action)
:Screen(gui),editorView(sf::FloatRect({0, 0}, {0,0}))
    , editorManager(editorState,gui)
{

   setupUI(actions);
   editorManager.setupUI(editorView);
   if(action=="Edit"){
    //openFile
    auto targetDir = std::filesystem::path("output/topologies");
    auto openFileDialog = tgui::FileDialog::create("Open file", "Open");
    openFileDialog->setMultiSelect(false);
    openFileDialog->setPath(targetDir.string());  // Set default dir
    openFileDialog->setFileTypeFilters({ {"Simulation File", {"*.simcfg"}} }, 1);
    openFileDialog->onFileSelect([this,&gui](const std::vector<tgui::Filesystem::Path>& paths){
       
        if (!paths.empty()) {
            if(TopologyConfigIO::read(paths[0].asString().toStdString(), editorState)){
                auto currentMode = editorState.getTopologyMode();
                std::string label = std::string(magic_enum::enum_name(currentMode));
                if (editorManager.modeDropdown->contains(label)) editorManager.modeDropdown->setSelectedItem(label);
                   
            }
            else{
                auto errorBox = UIFactory::createMessageBox("Error", "Failed to load topology configuration.");
                errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                    if(button == "OK" ){
                        msgBox->getParent()->remove(msgBox->shared_from_this()); 
                    } 
                });
                gui.add(errorBox);
            }

        }
    });
    openFileDialog->setTitle("Open Topology Configuration");
    openFileDialog->setSize({"50%", "50%"});
    openFileDialog->setPosition({"25%", "25%"});
    gui.add(openFileDialog);
    unsigned int textSize = 0.015f * gui.getView().getRect().height;  // e.g. 2.5% of height
    openFileDialog->setTextSize(textSize);
    openFileDialog->getRenderer()->setTextSize(textSize);
    }
    
}   

void TopologyEditorScreen::setupUI(std::vector<std::pair<std::string, ScreenAction>> actions)
{   
    gui.removeAllWidgets();

    updateTextSize(0.03f);

    auto picture = tgui::Picture::create("assets/background.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);

    auto button = UIFactory::createButton("Back", actions[0].second);
    button->setSize({"7%", "4%"});
    button->setPosition({"1%", "1%"});
    gui.add(button);
}



void TopologyEditorScreen::handleEvent(InputManager& input)
{
    
    if (input.isKeyPressed(sf::Keyboard::Scancode::Left)) {
        editorView.move({-20.f, 0.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Right)) {
        editorView.move({20.f, 0.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Up)) {
        editorView.move({0.f, -20.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Down)) {
        editorView.move({0.f, 20.f});
    }

    float wheelDelta = input.getMouseWheelDelta();
    if (wheelDelta > 0.f) {
        editorView.zoom(0.9f);
    } else if (wheelDelta < 0.f) {
        editorView.zoom(1.1f);
    }

    //todo: move in update?
    editorManager.handleInput(input);

}

void TopologyEditorScreen::update(float deltaTime, InputManager& input)
{


    // Example: Update model visualization, nodes, links, etc.
    //we don´t need delta time for now because there is no general animation.
    editorManager.update(deltaTime);
}


void TopologyEditorScreen::draw(sf::RenderWindow& window)
{
    // Switch back to default view for UI buttons
    editorManager.draw(window,editorView);
}

void TopologyEditorScreen::onResize()
{
    updateTextSize(0.03f);  // Update text size based on new window dimensions
    auto size =editorManager.canvas->getSize();
    editorView.setSize(size);
    editorView.setCenter(size / 2.f);
    editorManager.canvas->setView(editorView);
}
