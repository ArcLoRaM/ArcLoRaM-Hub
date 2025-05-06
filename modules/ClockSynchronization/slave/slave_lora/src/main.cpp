#include <Arduino.h>



// Turns the 'PRG' button into the power button, long press is off 
#define HELTEC_POWER_BUTTON   // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>

#include <esp_now.h>
#include <WiFi.h>

// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE               1800

// Frequency in MHz. Keep the decimal point to designate float.
// Check your own rules and regulations to see what is legal where you are.
#define FREQUENCY           866.3       // for Europe
// #define FREQUENCY           905.2       // for US

// LoRa bandwidth. Keep the decimal point to designate float.
// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH           250.0

// Number from 5 to 12. Higher means slower but higher "processor gain",
// meaning (in nutshell) longer range and more robust against interference. 
#define SPREADING_FACTOR    9

// Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be
// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER      0

String rxdata;
volatile bool rxFlag = false;
long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;
// Can't do Serial or display things here, takes too much time for the interrupt
void rx() {
  rxFlag = true;
}


//---------------------------------------------------------------------------------------------------
//Clock Syncrhonization
// Variables to store time and offset
unsigned long slaveTimeOffset = 0;      // Offset to synchronize with the master
float driftCorrectionFactor = 1.0;     // Correction factor for clock drift
float alpha = 0.1;
unsigned long lastSyncTime = 0;        // The millis() value when the last sync occurred
unsigned long lastMasterTime = 0;      // Master time received during the last sync


// Function to get the current slave time (with drift correction)
unsigned long getSlaveTime() {
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - lastSyncTime;

  // Apply drift correction to elapsed time
  //this line caused the error
  //unsigned long correctedElapsedTime = (unsigned long)(elapsedTime * driftCorrectionFactor);

  //we need to make sure all calculations are done in floating point
  unsigned long correctedElapsedTime = static_cast<unsigned long>(elapsedTime * (double)driftCorrectionFactor);

  both.print("elap:");
  both.println(elapsedTime);
  both.print("drift:");
  both.println(driftCorrectionFactor,6);
  both.print("corr:");
  both.println(correctedElapsedTime);

  // Return the adjusted slave time
  return lastMasterTime + correctedElapsedTime;
}


// Function to synchronize slave clock to master time
void synchronizeClock(unsigned long masterTime) {
  unsigned long currentMillis = millis();

  // Calculate elapsed time since the last sync
  if (lastSyncTime != 0) {
    unsigned long elapsedTime = currentMillis - lastSyncTime;
    unsigned long elapsedMasterTime = masterTime - lastMasterTime;

    // Update the drift correction factor´

    //Simple pattern
    // driftCorrectionFactor = (float)elapsedMasterTime / elapsedTime;

    //smoothing factor
    // Calculate the new drift factor
    float newDrift = (float)elapsedMasterTime / elapsedTime;

    // Apply smoothing to update drift correction factor
    driftCorrectionFactor = alpha * newDrift + (1 - alpha) * driftCorrectionFactor;
  }

  // Update synchronization variables
  lastSyncTime = currentMillis;
  lastMasterTime = masterTime;
}

//As all the nodes are located on a 1 meter wide table, we need to think of a strategy to simulate the distance between the nodes.
//every nodes have an id, and a listening id (the id of the only node that it listens to), other nodes are ignored.
String personalId = "3";
String listeningId = "2";
 String delimiter = ":";
unsigned long transmissionTime = 151; //ms, the time between the buffer is constructed and packet is received (we consider instantaneous transmission (but still significant Time On Air))

//---------------------------------------------------------------------------------------------------

//Callback to print the current time:
void onReceive(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  both.println("Data received");
    if (incomingData[0] == 0x01) { // Example: Command byte for "log time"
        unsigned long currentTime = getSlaveTime();
        both.printf("Clock: %lu \n", currentTime); // Log or save the time
    }
}











void setup() {
  heltec_setup();
  both.println("Radio init");
  RADIOLIB_OR_HALT(radio.begin());
  // Set the callback function for received packets
  radio.setDio1Action(rx);
  // Set radio parameters
  both.printf("Frequency: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  both.printf("Bandwidth: %.1f kHz\n", BANDWIDTH);
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  both.printf("TX power: %i dBm\n", TRANSMIT_POWER);
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  // Start receiving
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));


  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
      both.printf("Error initializing ESP-NOW\n");
      return;
  }
  esp_now_register_recv_cb(onReceive);
      both.printf("Callback registered\n");

}

void loop() {
  heltec_loop();
  
  bool tx_legal = millis() > last_tx + minimum_pause;
  // Transmit a packet every PAUSE seconds or when the button is pressed
  if ((PAUSE  && millis() - last_tx > (PAUSE * 1000)) || button.isSingleClick()) {
    

    char buffer[20];//the buffer to store the time, big enough!
    // Convert unsigned long to char*
    tx_time = millis();
    sprintf(buffer, "%lu", getSlaveTime()); // %lu is the format specifier for unsigned long
    strcat(buffer, delimiter.c_str());
    strcat(buffer, personalId.c_str());
    both.printf("TX [%s] ", buffer);
    radio.clearDio1Action();
    heltec_led(50); // 50% brightness is plenty for this LED
  
    RADIOLIB(radio.transmit(buffer));
    tx_time = millis() - tx_time;
    heltec_led(0);
    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      both.printf("OK (%i ms)\n", (int)tx_time);
    } else {
      both.printf("fail (%i)\n", _radiolib_status);
    }
    // Maximum 1% duty cycle
    minimum_pause = tx_time * 100;
    last_tx = millis();
    radio.setDio1Action(rx);
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }




  // If a packet was received, display it and the RSSI and SNR
  if (rxFlag) {
    rxFlag = false;
    radio.readData(rxdata);
    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      both.printf("RX [%s]\n", rxdata.c_str());
      String longNumber;
      int delimiterIndex = rxdata.indexOf(':');
      if (delimiterIndex != -1) {
        String senderId = rxdata.substring(delimiterIndex + 1);
        if(senderId!=listeningId){
          both.printf("Message from %s ignored\n", senderId.c_str());
          return;
        }
        longNumber = rxdata.substring(0, delimiterIndex);
      } else {
          both.printf("Delimiter not found");
      }
      //convert the received data to a long
      unsigned long timeRef=strtoul(longNumber.c_str(), nullptr, 10);

      //adjust clock:
      synchronizeClock(timeRef);

      //print slave time: 
      both.printf("Slave time: %lu\n", getSlaveTime());

      // both.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
      // both.printf("  SNR: %.2f dB\n", radio.getSNR());
    }
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }
}

