#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
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
  Serial.printf("Temperatur value: %g \n", boardsStruct[myData.id-1].x);
  Serial.printf("Humidity value: %g \n", boardsStruct[myData.id-1].y);
  Serial.println();
}
 
void setup() {
  Serial.begin(115200);
  setupLedPanel(); 
  setupReceivingData(); 
  xTaskCreatePinnedToCore(
    receivingData
    ,  "ReceivingData"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    ledPanel
    ,  "LedPanel"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}

void setupLedPanel() {
  // Do nothing right now
}

void setupReceivingData() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void ledPanel(void *pvParameters) {
   (void) pvParameters;
   for (;;) {
    // do nothing right now
    Serial.println("LED-Panel");
    vTaskDelay(2500);
   }
}

void receivingData(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    // Acess the variables for each board
    float board1X = boardsStruct[0].x;
    float board1Y = boardsStruct[0].y;
    vTaskDelay(5000);
  }
}

void loop() {
  // Do nothing
}
