# arcloram-visualisation

ArcLoRaM Visualizer is part of a larger project available at https://github.com/Rorschak84/ArcLoRaM-Hub , please consult the linked report for a total explanation of the protocol whereabouts and implementations.

The original idea behind the visualizer is to propose an interface to interact with the simulator as it is quite cumbersome to manually implement various topologies and understand what's going on in the network through text logs. The visualizer significantly reduces the time required to test and refine the protocol by offering an accessible, visual interface for debugging and performance analysis. 

## Demo:

[Link to the video
](https://youtu.be/G0fd1k9Jc2k?si=-FvLKjObjD7HH-DO)

## Installation

The simplest way to run the protocol is by using docker. 
(on its way...)

However, if you wish to improve the vizualiser, it is better to use it with an IDE as it enables live debugging, performance check..
I personnaly used Visual Studio Code with the latest C++ extension.
The SFML library is required for the visualizer and the simulator. Make sure to use the correct version that match your compiler! 
Consult the following video to install it with visual studio code: https://www.youtube.com/watch?v=rZE700aaT5I&t=429s
Also Possible on Visual studio: https://www.youtube.com/watch?v=lFzpkvrscs4&ab_channel=EOD-Ethan. 
Next step is to run the code. This of course depends on your local setup that might vary. The provided vscode folder provides instructions to run the code from the main.cpp file using visual studio on windows.

The visualizer is actually a server that runs on local host on port 5000, so make sure you don't have that port taken by another system.


