#include <Arduino.h>

#include <heltec_unofficial.h>//to have the helper functions for Screen and Button
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


void sendSyncSignal() {
    uint8_t message[] = {0x01}; // Sync signal
    esp_err_t result = esp_now_send(broadcastAddress, message, sizeof(message));
  if (result == ESP_OK) {
    both.println("Message sent successfully!");
  } else {
      both.print("Error sending message: ");
      both.println(result);
  }
}


void setup() {
  heltec_setup();
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
      both.printf("Error initializing ESP-NOW");
      return;
  }


      // Register the broadcast address as a peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6); // Copy the broadcast address
    peerInfo.channel = 0; // Default Wi-Fi channel (or set to match your network if using AP+STA mode)
    peerInfo.encrypt = false; // No encryption for simplicity

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        both.println("Failed to add broadcast peer.");
        return;
    }
    both.println("Broadcast peer added.");
  both.println("Setup Done");






}

void loop() {
  heltec_loop();

  if(button.isSingleClick()) {
    sendSyncSignal();
    
  }
}
