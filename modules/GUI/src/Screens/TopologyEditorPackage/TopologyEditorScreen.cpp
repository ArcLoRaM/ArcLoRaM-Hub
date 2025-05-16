#include "TopologyEditorScreen.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"


TopologyEditorScreen::TopologyEditorScreen(ScreenAction backAction)
    : editorView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    initializeUI(backAction);
    // TopologyModel stays empty for new topology
}

TopologyEditorScreen::TopologyEditorScreen(const std::filesystem::path& topologyFilePath, ScreenAction backAction)
    : editorView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    initializeUI(backAction);
    // loadTopologyFromFile(topologyFilePath);
}


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
        "Arial"
    );
    backButton->setOnClick(backAction);

    posY += buttonHeight + spacingY;

    saveButton = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 200, 50),
        "Save",
        "Arial"
    );
    saveButton->setOnClick([this]() {
        // TODO: Save functionality
    });

    posY += buttonHeight + spacingY;

    addNodeButton = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 150, 250),
        "Add Node",
        "Arial"
    );
    addNodeButton->setOnClick([this]() {
        mode = EditorMode::AddingNode;
    });

    posY += buttonHeight + spacingY;

    addLinkButton = std::make_unique<Button>(
        startX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(50, 150, 250),
        "Add Link",
        "Arial"
    );
    addLinkButton->setOnClick([this]() {
        mode = EditorMode::AddingLink;
    });
}

void TopologyEditorScreen::handleEvent(InputManager& input)
{
    // Update buttons for hover/click detection
    if (backButton) backButton->update(input);
    if (saveButton) saveButton->update(input);
    if (addNodeButton) addNodeButton->update(input);
    if (addLinkButton) addLinkButton->update(input);

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
}

void TopologyEditorScreen::update(float deltaTime, InputManager& input)
{
    (void)deltaTime;
    (void)input;

    // Example: Update model visualization, nodes, links, etc.
    // Currently no dynamic elements to update yet.
}


void TopologyEditorScreen::draw(sf::RenderWindow& window)
{
    // Set to editor view (for zoom/pan)
    window.setView(editorView);

    // TODO: Draw topology nodes, links, etc. here

    // Switch back to default view for UI buttons
    window.setView(window.getDefaultView());

    if (backButton) backButton->draw(window);
    if (saveButton) saveButton->draw(window);
    if (addNodeButton) addNodeButton->draw(window);
    if (addLinkButton) addLinkButton->draw(window);
}
