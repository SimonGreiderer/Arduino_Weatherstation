#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 4     
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

uint8_t broadcastAddress[] = {0x4C, 0x11, 0xAE, 0xCB, 0x6D, 0x74};

typedef struct struct_message {
    int id; // must be unique for each sender board
    float x; // Temperatur
    float y; // Humidity
} struct_message;

DHT dht(DHTPIN, DHTTYPE);
struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  dht.begin();
}

void loop() {
  myData.id = 1;
  myData.x = dht.readTemperature();
  myData.y = dht.readHumidity();

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(5000);
}
