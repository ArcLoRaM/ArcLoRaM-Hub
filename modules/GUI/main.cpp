#include "Button/Button.hpp"
#include "VisualiserManager/VisualiserManager.hpp"
#include <iostream>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include "Packets/Packets.hpp"
#include "Common/Common.hpp"
#include "network.cpp"
#include "display.cpp"




int main() {
    
    
    VisualiserManager manager;
    // Create buttons
    //TODO: get rid of useless elements like on off
    // std::unique_ptr<Button> button1 =std::make_unique <Button> (x1coor, y1coor, width1, height1, color1, state1, on, off, iconPath1);
    // std::unique_ptr<Button> button2 =std::make_unique <Button> (x2coor, y2coor, width2, height2, color2, state2, on, off, iconPath2);
    // std::unique_ptr<Button> button3 =std::make_unique <Button> (x3coor, y3coor, width3, height3, color3, state3, on, off, iconPath3);
    std::unique_ptr<Button> button4 =std::make_unique <Button> (x4coor, y4coor, width4, height4, color4, state4, on, off, iconPath4);


    // manager.addButton(std::move(button1));
    // manager.addButton(std::move(button2));
    // manager.addButton(std::move(button3));
    manager.addButton(std::move(button4));

    // Start threads
    std::thread network(networkThread,std::ref (manager));
    std::thread display(displayThread, std::ref(manager));


    // Wait for threads to finish
    network.join();
    display.join();

    return 0;
}
