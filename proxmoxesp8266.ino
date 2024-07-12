#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

const char* proxmoxApiUrl = "https://your_proxmox_server/api2/json/nodes/your_node/status";
const char* token = "your_api_token";

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(proxmoxApiUrl);
    http.addHeader("Authorization", String("PVEAPIToken=") + token);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      const char* nodeName = doc["data"]["name"];
      float cpuUsage = doc["data"]["cpu"];
      uint64_t memoryTotal = doc["data"]["memory"]["total"];
      uint64_t memoryUsed = doc["data"]["memory"]["used"];
      uint64_t uptime = doc["data"]["uptime"];

      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.print("Node: ");
      display.println(nodeName);

      display.print("CPU Usage: ");
      display.print(cpuUsage * 100);
      display.println("%");

      display.print("Memory Used: ");
      display.print(memoryUsed / 1024 / 1024);
      display.print("/");
      display.print(memoryTotal / 1024 / 1024);
      display.println(" MB");

      display.print("Uptime: ");
      display.print(uptime / 3600);
      display.println(" hrs");

      display.display();
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
  delay(10000);
}
