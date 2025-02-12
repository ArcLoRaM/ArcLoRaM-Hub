# ArcLoRaM-Hub
ArcLoRaM (Arctic LoRa Mesh) is an innovative protocol tailored for large-scale environment monitoring with an emphasis on ultra low power consumption
The proposed protocol, code and report have been made over a period of 5 months, in the context of the obtention of my master thesis.


ArcLoRaM employs a semi-distributed mesh network architecture to minimize interference and enable an efficient multi-hop relay mechanism. This design facilitates coverage over areas potentially exceeding tens of thousands of square kilometers with a maximum of 1,024 devices. The protocol’s core focus is power efficiency, achieved through a power-aware routing mechanism and a Time-Division Multiple Access (TDMA) communication scheme to delay the apparition of the first node failure. Data integrity is ensured through built-in reliability mechanisms, including acknowledgments and per-link guarantees. Additionally, the protocol features a simple authentication scheme to prevent unauthorized access to the network

In its current state, this repository holds three sub-projects: 
-a simulator that implements 3 out of the 6 ArcLoRaM's modes
-a visualiser to display logged elements and understand how the packets are flowing
-a clock synchronization algorithm.

A complete explanation of the protocol features and mechanisms, sub project description and results can be found in the available report of approximately 50 pages.
A video highlighting protocol key functionnalities of the protocol thanks to the visualizer is available at :



Contact Information:
-simlanglais@gmail.com

You're very welcome to fork and use this code freely. Pull requests are also desirable. I am no longer working in this project, but my university might pursue its development.
