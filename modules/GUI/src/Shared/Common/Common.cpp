#include "Common.hpp"


//shared data
std::vector<std::string> logMessages;  // Shared container for received strings
std::mutex logMutex;                  // Mutex to protect the container
bool isRunning = true;                // Flag to signal threads to stop
std::string COMMUNICATION_MODE = "None";
int TICK_NB = 0;
float DISTANCE_THRESHOLD = 0.0f;
std::mutex deviceMutex;

constexpr const  int windowWidth = 1600;
constexpr const  int windowHeight = 1000;

int NBRETRANSMISSION=0;
int ENERGYEXP=0;

int verticalOffset=1100;
int horizontalOffset=800;
float distanceDivider=0.4f;

//Buttons:
const std::string on = "ON";
const std::string off = "OFF";


const float x4coor = 1800.f - 350.f;
const float y4coor = 100.f;
const float width4 = 80.f;
const float height4 = 40;
const sf::Color color4 = sf::Color::Yellow;
std::string state4 = off;
const std::string iconPath4 = "assets/Icons/routing.png";


//Animations: 
const sf::Color dataArrowColor = sf::Color::Red;
const sf::Color ackArrowColor = sf::Color::Green;

float broadcastDuration = 1.3f;
float receptionDuration = 1.0f;
float arrowDuration = 0.7f;
double radiusIcon=35.0f;