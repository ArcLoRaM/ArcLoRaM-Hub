# ArcLoRaM-ClockSynchronization

This project is part of the larger "ArcLoRaM" initiative that develops a LoRa Mesh Protocol for large scale monitoring of the environment. (https://github.com/Rorschak84/ArcLoRaM-Hub).
Its focus is on developping a synchronizing clock algorithm through LoRa Communication and establish a way to measure internal clock value.
More informations are available in the main report present in the ArcLoRaM Hub, in the appropriate section.
The codes uses the arduino framework on the Heltech Wifi Lora V3.

Three sources codes are available:
-Master: clock value reference for the slaves devices. Broadcast a beacon containing an Id at regular interval.
-Slave: Upon Receiving a beacon, they will synchronize their clock value and forward the beacon with their own id. Every slave is listening to only one other device ID, to test various topology easily.
-Notifier: When the user press the button on the notifier board, it will use a wireless technology called ESP-NOW that triggers every other devices to print their synchronized interal clock value, allowing easy comparison.


#Installation

Before using the code, have a look on the fantastic library available at: https://github.com/ropg/heltec_esp32_lora_v3

I personnaly used it with platform io as It requires only one line of code to add in the platformio.ini for the library to be installed.
However, I noticed there is a compile error when utilizing more than one source file. I tried to install the library manually to bypass this problem, without success.


#Algorithm And Result

Please, consult the report in the Hub Page for detailed explanation.



If you have any questions, you're welcome to join me at : simlanglais@gmail.com





