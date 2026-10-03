#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- Change these ----
const char* WIFI_SSID     = "ABCD";                                     //Wifi Name
const char* WIFI_PASSWORD = "1234";                              // Wifi Pasward
// ----------------------

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C   // try 0x3D if the screen stays blank

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int rssiToPercent(int rssi) {
  if (rssi <= -100) return 0;
  if (rssi >= -50)  return 100;
  return 2 * (rssi + 100);
}

int rssiToBars(int rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -67) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

const char* rssiToLabel(int rssi) {
  if (rssi >= -55) return "Excellent";
  if (rssi >= -67) return "Good";
  if (rssi >= -75) return "Fair";
  if (rssi >= -85) return "Weak";
  return "Very weak";
}

void drawBars(int x, int y, int bars) {
  for (int i = 0; i < 4; i++) {
    int h = 6 + i * 5;
    int bx = x + i * 8;
    int by = y + 26 - h;
    if (i < bars) display.fillRect(bx, by, 6, h, SSD1306_WHITE);
    else          display.drawRect(bx, by, 6, h, SSD1306_WHITE);
  }
}

void showMessage(const char* line1, const char* line2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.println(line2);
  display.display();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  showMessage("Connecting to:", WIFI_SSID);

  unsigned long start = millis();
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    display.setCursor(dots * 6, 30);
    display.print(".");
    display.display();
    dots = (dots + 1) % 20;
  }

  if (WiFi.status() == WL_CONNECTED) {
    showMessage("Connected!", WiFi.localIP().toString().c_str());
    delay(1000);
  } else {
    showMessage("WiFi failed", "Retrying...");
    delay(1500);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 not found - check wiring/address");
    while (true) delay(1000);
  }

  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    showMessage("Disconnected", "Reconnecting...");
    WiFi.disconnect();
    connectWiFi();
    return;
  }

  int rssi    = WiFi.RSSI();
  int percent = rssiToPercent(rssi);
  int bars    = rssiToBars(rssi);

  Serial.printf("RSSI: %d dBm (%d%%)\n", rssi, percent);

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(WiFi.SSID());

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(rssi);
  display.setTextSize(1);
  display.print(" dBm");

  display.setCursor(0, 38);
  display.printf("%d%%  %s", percent, rssiToLabel(rssi));

  drawBars(94, 10, bars);

  display.drawRect(0, 54, 128, 10, SSD1306_WHITE);
  display.fillRect(2, 56, (124 * percent) / 100, 6, SSD1306_WHITE);

  display.display();
  delay(500);
}