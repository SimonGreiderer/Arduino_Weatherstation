#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontRobotron.h>
#include <ThingsBoard.h>

#define LED_PIN        4
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  -8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX // Wie sind die LEDs angeordnet

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define WIFI_AP_NAME        "SSID"
#define WIFI_PASSWORD       "PW"
#define TOKEN               "Weatherstation"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;
bool subscribed = false;

// MAC: 4C:11:AE:CB:6D:74
typedef struct struct_message {
  int id;
  float x;
  float y;
} struct_message;

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText ScrollingMsg;
// Create a struct_message called myData
struct_message myData;
struct_message board1;
struct_message boardsStruct[1] = {board1};

const unsigned char TxtDemo[] = { EFFECT_SCROLL_LEFT EFFECT_HSV_CV "\x00\xff\xff\x40\xff\xff" "TEMPERATUR"};

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].x = myData.x;
  boardsStruct[myData.id - 1].y = myData.y;
  Serial.printf("Temperatur value: %g \n", boardsStruct[myData.id - 1].x);
  Serial.printf("Humidity value: %g \n", boardsStruct[myData.id - 1].y);
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
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(10); // WICHTIG - Hier wird die Helligkeit eingestellt. Am Anfang einen niedrigen Wert verwenden, und langsam hochtasten.
  FastLED.clear(true);
  delay(500);
  ScrollingMsg.SetFont(RobotronFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}

void setupReceivingData() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  boardsStruct[0].x = 0;
  boardsStruct[0].y = 0;
}

int calculatePixelNr(int x, int y) {
  return x % 2 == 1 ? 8 * x + 7 - y : 8 * x + y;
}

void InitWiFi() {
    Serial.println("Connecting to AP ...");
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
}

void reconnect() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(500);
    }
    Serial.println("Connected to AP");
  }
}

void ledPanel(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (ScrollingMsg.UpdateText() == -1) {
      unsigned char temperatur[17];
      sprintf((char *)temperatur, " WARM: %g", boardsStruct[0].x);
      ScrollingMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
      ScrollingMsg.SetScrollDirection(SCROLL_LEFT);
      ScrollingMsg.SetTextColrOptions(COLR_HSV | COLR_GRAD_CH, 0x00, 0xff, 0xff, 0x40, 0xff, 0xff);
      ScrollingMsg.SetText(temperatur, sizeof(temperatur) - 1);
      vTaskDelay(20);
    }
    else
      FastLED.show();
    vTaskDelay(20);
  }
}

void receivingData(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    setupReceivingData();
    // Acess the variables for each board
    float board1X = boardsStruct[0].x;
    float board1Y = boardsStruct[0].y;
    sendingDataThingsboard(board1X, board1Y);
    vTaskDelay(5000);
  }
}

void sendingDataThingsboard(float temp, float hum) {
  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  InitWiFi();
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }
  if (!tb.connected()) {
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }
  Serial.println("Sending data...");
  tb.sendTelemetryFloat("temperature", temp);
  tb.sendTelemetryFloat("humidity", hum);
}

void loop() {
  // Do nothing
}
