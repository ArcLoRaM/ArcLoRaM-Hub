#include "TopologyEditorScreen.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"
#include "TopologyConfigIO.hpp"
#include "../../UI/UIFactory/UIFactory.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>

TopologyEditorScreen::TopologyEditorScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
    : Screen(gui),editorView(sf::FloatRect({0, 0}, {0,0}))
    , editorManager(editorState,gui)
{
    setupUI(actions);
    editorManager.setupUI(gui,editorView);
}

TopologyEditorScreen::TopologyEditorScreen(const std::string &topologyFilePath,std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
:Screen(gui),editorView(sf::FloatRect({0, 0}, {0,0}))
    , editorManager(editorState,gui)
{

   setupUI(actions);
   editorManager.setupUI(gui,editorView);
   TopologyConfigIO::read(topologyFilePath,editorState);

}

void TopologyEditorScreen::setupUI(std::vector<std::pair<std::string, ScreenAction>> actions)
{   
    gui.removeAllWidgets();

    updateTextSize(0.03f);

    auto picture = tgui::Picture::create("assets/background.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);



    constexpr float buttonWidth = 150.f;
    constexpr float buttonHeight = 50.f;
    constexpr float spacingY = 20.f;
    constexpr float startX = 20.f;

    float posY = 20.f;


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

    editorManager.handleInput(input);

}

void TopologyEditorScreen::update(float deltaTime, InputManager& input)
{
    (void)deltaTime;
    (void)input;

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
