#include <Arduino.h>
/**
 * Send and receive LoRa-modulation packets with a sequence number, showing RSSI
 * and SNR for received packets on the little display.
 *
 * Note that while this send and received using LoRa modulation, it does not do
 * LoRaWAN. For that, see the LoRaWAN_TTN example.
 *
 * This works on the stick, but the output on the screen gets cut off.
*/



// Turns the 'PRG' button into the power button, long press is off 
#define HELTEC_POWER_BUTTON   // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>

#include <esp_now.h>
#include <WiFi.h>

// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE               3600

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


String personalId = "0";//packets are adressed to simulate distance between nodes
String delimiter = ":";
unsigned long transmissionTime = 152; //ms, the time between the buffer is constructed and packet is received (we consider instantaneous transmission (but still significant Time On Air))
//---------------------------------------------------------------------------------------------------


//CallBack function for recording internal clock


void onReceive(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
    if (incomingData[0] == 0x01) { // Example: Command byte for "log time"
        unsigned long currentTime = millis();
        both.printf("Clock: %lu \n", currentTime);
    }
}

//---------------------------------------------------------------------------------------------------







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
  if ((PAUSE/* && tx_legal*/ && millis() - last_tx > (PAUSE * 1000)) || button.isSingleClick()) {


    // In case of button click, tell user to wait
    // if (!tx_legal) {
    //   both.printf("Legal limit, wait %i sec.\n", (int)((minimum_pause - (millis() - last_tx)) / 1000) + 1);
    //   return;
    // }

    char buffer[20];//the buffer to store the time, big enough!
    // Convert unsigned long to char*
    tx_time = millis();
    sprintf(buffer, "%lu", tx_time+transmissionTime); // %lu is the format specifier for unsigned long
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


  //Master do not need to receive


  // // If a packet was received, display it and the RSSI and SNR
  // if (rxFlag) {
  //   rxFlag = false;
  //   radio.readData(rxdata);
  //   if (_radiolib_status == RADIOLIB_ERR_NONE) {
  //     both.printf("RX [%s]\n", rxdata.c_str());
      
  //   }
  //   RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  // }


}

