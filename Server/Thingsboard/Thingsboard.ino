#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#include <WiFi.h>
#include <ThingsBoard.h>

#define LED_PIN        4
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  -8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX // Wie sind die LEDs angeordnet

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define WIFI_AP_NAME        "TheHotspot"
#define WIFI_PASSWORD       "Aot>Arcane"
#define TOKEN               "Weatherstation"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;
bool subscribed = false;
int grenzwert = 25; 

void setup() {
  Serial.begin(115200);
}


void InitWiFi() {
    Serial.println("Connecting to AP ...");
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
}

void reconnect() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    Serial.println("Connected to AP");
  }
}

void sendingDataThingsboard(float temp, float hum) {
  WiFi.disconnect(); 
  InitWiFi();
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }
  if (!tb.connected()) {
    Serial.println("Not connected yet"); 
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
    Serial.println("Connected"); 
  }
  Serial.println("Sending data...");
  tb.sendTelemetryFloat("temperature", temp);
  tb.sendTelemetryFloat("humidity", hum);
  Serial.println("Data has been sent"); 
  WiFi.disconnect(); 
}

void loop() {
  sendingDataThingsboard(20.0, 50.0);
  delay(5000); 
}
