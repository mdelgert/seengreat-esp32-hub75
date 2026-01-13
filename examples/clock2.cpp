#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <time.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 1

// ========== WiFi + NTP Setup ==========
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ===== PIN MAPPING for ESP32-S3 DevKitC-1 Matrix Adapter =====
HUB75_I2S_CFG::i2s_pins pins = {
  .r1 = 37,
  .g1 = 6,
  .b1 = 36,
  .r2 = 35,
  .g2 = 5,
  .b2 = 0,
  .a  = 45,
  .b  = 1,
  .c  = 48,
  .d  = 2,
  .e  = -1,    // 64×32 → 1/16 scan → NO E pin
  .lat = 38,
  .oe  = 21,
  .clk = 47
};

// Panel config
HUB75_I2S_CFG mxconfig(
    PANEL_WIDTH,
    PANEL_HEIGHT,
    PANELS_NUMBER,
    pins
);

MatrixPanel_I2S_DMA *matrix = nullptr;

// ======= 7-SEGMENT CLOCK FONT (CUSTOM DRAWING) =======
// Each digit is 16 px tall, 10 px wide (fits 32-pixel height nicely)

void drawSegment(int x, int y, bool on) {
  uint16_t color = on ? matrix->color565(255, 0, 0) : matrix->color565(10, 0, 0);
  matrix->fillRect(x, y, 8, 3, color);
}

void drawDigit(int x, int y, int digit) {
  // A B C D E F G segments
  bool s[7] = {0};

  switch (digit) {
    case 0: s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=1; break;
    case 1: s[1]=s[2]=1; break;
    case 2: s[0]=s[1]=s[6]=s[4]=s[3]=1; break;
    case 3: s[0]=s[1]=s[6]=s[2]=s[3]=1; break;
    case 4: s[5]=s[6]=s[1]=s[2]=1; break;
    case 5: s[0]=s[5]=s[6]=s[2]=s[3]=1; break;
    case 6: s[0]=s[5]=s[6]=s[4]=s[3]=s[2]=1; break;
    case 7: s[0]=s[1]=s[2]=1; break;
    case 8: s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=1; break;
    case 9: s[0]=s[1]=s[2]=s[3]=s[5]=s[6]=1; break;
  }

  // A
  if (s[0]) matrix->fillRect(x+1,  y,     10, 2, matrix->color565(255,0,0));
  // B
  if (s[1]) matrix->fillRect(x+11, y+1,   2, 10, matrix->color565(255,0,0));
  // C
  if (s[2]) matrix->fillRect(x+11, y+13,  2, 10, matrix->color565(255,0,0));
  // D
  if (s[3]) matrix->fillRect(x+1,  y+22, 10, 2, matrix->color565(255,0,0));
  // E
  if (s[4]) matrix->fillRect(x,    y+13, 2, 10, matrix->color565(255,0,0));
  // F
  if (s[5]) matrix->fillRect(x,    y+1,  2, 10, matrix->color565(255,0,0));
  // G
  if (s[6]) matrix->fillRect(x+1,  y+11, 10, 2, matrix->color565(255,0,0));
}

void drawColon(int x, int y) {
  matrix->fillRect(x, y+8,  3, 3, matrix->color565(255, 0, 0));
  matrix->fillRect(x, y+16, 3, 3, matrix->color565(255, 0, 0));
}

void setupTime() {
  configTime(-7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

// ========== MAIN SETUP ==========
void setup() {
  Serial.begin(115200);

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->setBrightness8(180);
  matrix->begin();

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  setupTime();
}

// ========== MAIN LOOP ==========
void loop() {
  matrix->fillScreenRGB888(0, 0, 0);  // clear screen

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  // Draw HH:MM - adjusted spacing for proper separation
  drawDigit(0, 3, hour / 10);       // Start at x=0
  drawDigit(14, 3, hour % 10);      // x=14 (0+13+1 spacing)
  drawColon(28, 3);                 // x=28 (14+13+1 spacing), colon is 3px wide
  drawDigit(35, 3, minute / 10);    // x=35 (28+3+4 spacing after colon)
  drawDigit(49, 3, minute % 10);    // x=49 (35+13+1 spacing) - fits within 64px (49+13=62)

  delay(500); // updates every half second
}
