#pragma once

#include <SFML/Graphics.hpp>
#include <magic_enum.hpp>
#include <vector>
#include <string>
#include <functional>

#include "../../Shared/InputManager/InputManager.hpp" 
#include "../../Shared/RessourceManager/RessourceManager.hpp"
//Header file for the Dropdown class as a template


template<typename EnumType>
class Dropdown {
public:
    static_assert(std::is_enum_v<EnumType>, "Dropdown only supports enum types.");

    using Callback = std::function<void(EnumType)>;

    Dropdown( float width, float itemHeight)
        :  itemHeight(itemHeight) {
        box.setSize({width, itemHeight});
        box.setFillColor(sf::Color(180, 180, 180));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::Black);

             font = &ResourceManager::getInstance().getFont("Arial");

        for (EnumType e : magic_enum::enum_values<EnumType>()) {
            items.push_back(e);

            sf::Text text(
                *font,
                std::string(magic_enum::enum_name(e)),
                
                static_cast<unsigned int>(itemHeight * 0.6f)
            );
            text.setFillColor(sf::Color::Black);
            itemTexts.push_back(std::move(text));
        }

        selectedIndex = 0;
    }

    void setPosition(sf::Vector2f pos) {
        position = pos;
        updateTexts();
    }

    void setCallback(Callback cb) {
        onSelect = std::move(cb);
    }

    void update(const InputManager& input) {
        if (!input.isMouseJustPressed()) return;

        sf::Vector2f mousePos = input.getMouseUIScreenPosition();

        if (expanded) {
            for (std::size_t i = 0; i < items.size(); ++i) {
                sf::FloatRect itemRect({position.x,
                                       position.y + itemHeight * static_cast<float>(i)},
                                       {box.getSize().x,
                                       itemHeight});
                if (itemRect.contains(mousePos)) {
                    selectedIndex = i;
                    expanded = false;
                    if (onSelect) onSelect(items[i]);
                    updateTexts();
                    return;
                }
            }
        } else {
            sf::FloatRect baseRect(position, {box.getSize().x, itemHeight});
            if (baseRect.contains(mousePos)) {
                expanded = true;
            } else {
                expanded = false;
            }
        }
    }

void draw(sf::RenderWindow& window) {
    if (expanded) {
        // Draw selected item at the top (highlighted)
        box.setPosition(position);
        window.draw(box);

        sf::Text selectedText = itemTexts[selectedIndex];
        selectedText.setPosition({position.x + 5.f, position.y});
        selectedText.setFillColor(sf::Color::White);
        box.setFillColor(sf::Color(100, 100, 255)); // highlighted background
        window.draw(box);
        window.draw(selectedText);

        // Reset box color for others
        box.setFillColor(sf::Color(180, 180, 180));

        float yOffset = itemHeight;
        for (std::size_t i = 0; i < items.size(); ++i) {
            if (i == selectedIndex) continue; // skip already drawn selected item

            box.setPosition(sf::Vector2f(position.x, position.y + yOffset));
            itemTexts[i].setPosition({position.x + 5.f, position.y + yOffset});
            window.draw(box);
            window.draw(itemTexts[i]);

            yOffset += itemHeight;
        }
    } else {
        // Collapsed: draw only selected item
        box.setPosition(position);
        window.draw(box);

        sf::Text selectedText = itemTexts[selectedIndex];
        selectedText.setPosition({position.x + 5.f, position.y});
        window.draw(selectedText);
    }

    // Reset box position and color
    box.setPosition(position);
    box.setFillColor(sf::Color(180, 180, 180));
}



    EnumType getSelected() const {
        return items[selectedIndex];
    }

    void setSelected(EnumType value) {
        auto it = std::find(items.begin(), items.end(), value);
        if (it != items.end()) {
            selectedIndex = std::distance(items.begin(), it);
            updateTexts();
        }
    }

private:
    sf::RectangleShape box;
    float itemHeight;
    sf::Vector2f position;

    std::vector<EnumType> items;
    std::vector<sf::Text> itemTexts;
    std::size_t selectedIndex = 0;
    bool expanded = false;
    const sf::Font* font = nullptr;

    Callback onSelect;

    void updateTexts() {
        for (std::size_t i = 0; i < itemTexts.size(); ++i) {
            itemTexts[i].setPosition({position.x + 5, position.y + itemHeight * static_cast<float>(i)});
        }
    }
};
