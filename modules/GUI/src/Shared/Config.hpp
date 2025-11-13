#pragma once
#include <SFML/Graphics.hpp>


    //TODO this file should be named: settings, or defaults
namespace config {

    //System
    inline int windowWidth = 1600;
    inline int windowHeight = 1000;

    //Animations 
    inline sf::Color dataArrowColor = sf::Color::Red;
    inline sf::Color ackArrowColor = sf::Color::Green;

    inline float broadcastDuration = 1.3f;//todo: remove it when start/end animation is implemented
    inline float receptionDuration = 1.0f;
    inline float arrowDuration = 0.7f;
    inline double radiusIcon=70.0f;
    inline float distanceThreshold = 1000.f; // Distance threshold for the broadcast animation
}