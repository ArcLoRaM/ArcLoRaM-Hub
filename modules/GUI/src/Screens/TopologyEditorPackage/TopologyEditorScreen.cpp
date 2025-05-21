#include "TopologyEditorScreen.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"


TopologyEditorScreen::TopologyEditorScreen(ScreenAction backAction)
    : editorView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
    , editorManager(editorState)
{
    initializeUI(backAction);
    // TopologyModel stays empty for new topology
}

// TopologyEditorScreen::TopologyEditorScreen(const std::filesystem::path& topologyFilePath, ScreenAction backAction)
//     : editorView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
// {
//     initializeUI(backAction);
//     // loadTopologyFromFile(topologyFilePath);
// }


void TopologyEditorScreen::initializeUI(ScreenAction backAction)
{
    constexpr float buttonWidth = 150.f;
    constexpr float buttonHeight = 50.f;
    constexpr float spacingY = 20.f;
    constexpr float startX = 20.f;

    float posY = 20.f;

    backButton = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(200, 50, 50),
        "Back",
        "Arial",
        false
    );
    backButton->setOnClick(backAction);

    
}

void TopologyEditorScreen::handleEvent(InputManager& input)
{
    // Update buttons for hover/click detection
    if (backButton) backButton->update(input);

    // Example of future navigation keys (like in ProtocolVisualisationScreen)
    if (input.isKeyPressed(sf::Keyboard::Scancode::Left)) {
        editorView.move({-10.f, 0.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Right)) {
        editorView.move({10.f, 0.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Up)) {
        editorView.move({0.f, -10.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Down)) {
        editorView.move({0.f, 10.f});
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
    window.setView(window.getDefaultView());
    if (backButton) backButton->draw(window);
    editorManager.draw(window,editorView);
}
