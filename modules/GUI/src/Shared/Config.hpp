#pragma once
#include <SFML/Graphics.hpp>

namespace config {

    //System
    inline int windowWidth = 1600;
    inline int windowHeight = 1000;


    //UI
    //Do we really need those since we have the zoom/dezoom plus moving camera? -> when you will create new topologies, it will be complex to handle this no?
    inline int verticalOffset=1100;
    inline int horizontalOffset=800;
    inline float distanceDivider=0.4f;


    inline int maxLogMessages = 10; // Maximum number of log messages to keep in the log

    //Animations 
    inline sf::Color dataArrowColor = sf::Color::Red;
    //not used
    inline sf::Color ackArrowColor = sf::Color::Green;
    inline float broadcastDuration = 1.3f;
    inline float receptionDuration = 1.0f;
    inline float arrowDuration = 0.7f;
    inline double radiusIcon=35.0f;
    inline float distanceThreshold = 1000.f; // Distance threshold for the broadcast animation

}