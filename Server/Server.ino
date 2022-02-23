
#include <esp_now.h>
#include <WiFi.h>


// MAC: 4C:11:AE:CB:6D:74
typedef struct struct_message {
  int id;
  float x;
  float y;
}struct_message;

// Create a struct_message called myData
struct_message myData;
struct_message board1;
struct_message boardsStruct[1] = {board1};

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  Serial.printf("Temperatur value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("Humidity value: %d \n", boardsStruct[myData.id-1].y);
  Serial.println();
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  // Acess the variables for each board
  float board1X = boardsStruct[0].x;
  float board1Y = boardsStruct[0].y;
  delay(5000);  
}
