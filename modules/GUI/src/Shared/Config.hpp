#pragma once
#include <SFML/Graphics.hpp>


    //TODO this file should be named: settings, or defaults
namespace config {

    //System
    inline int windowWidth = 1600;
    inline int windowHeight = 1000;


    //UI
    //Do we really need those since we have the zoom/dezoom plus moving camera? -> when you will create new topologies, it will be complex to handle this no?
    inline int verticalOffset=1100;
    inline int horizontalOffset=800;
    inline float distanceDivider=0.4f;


    //Animations 
    inline sf::Color dataArrowColor = sf::Color::Red;
    inline sf::Color ackArrowColor = sf::Color::Green;
    inline float broadcastDuration = 1.3f;//todo: remove it when start/end animation is implemented
    inline float receptionDuration = 1.0f;//todo: remove it when start/end animation is implemented
    inline float arrowDuration = 0.7f;
    inline double radiusIcon=35.0f;

    inline float distanceThreshold = 1000.f; // Distance threshold for the broadcast animation
}