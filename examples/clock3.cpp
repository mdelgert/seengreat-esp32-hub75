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


// ======= PROFESSIONAL 7-SEGMENT CLOCK DISPLAY =======
// Based on standard 7-segment display proportions and best practices
// Segment dimensions follow industry standard ratios: 
// - Height to width ratio of approximately 2:1
// - Segment thickness of ~10% of digit height
// - Proper gaps between segments for readability

#define DIGIT_WIDTH 10        // Individual digit width
#define DIGIT_HEIGHT 20       // Individual digit height (better aspect ratio)
#define SEGMENT_THICK 2       // Thickness of each segment
#define SEGMENT_GAP 1         // Gap between segments within a digit
#define DIGIT_SPACING 2       // Space between adjacent digits
#define COLON_WIDTH 2         // Width of colon dots
#define COLON_SPACING 4       // Space around colon for balance

// Colors for better contrast and readability
#define COLOR_ON   matrix->color565(0, 255, 0)    // Bright green for active segments
#define COLOR_COLON matrix->color565(0, 220, 0)   // Slightly dimmer colon

// Calculate total display width and center it
// Layout: H H : M M = 2 digits + colon + 2 digits + spacing
#define TOTAL_CLOCK_WIDTH ((4 * DIGIT_WIDTH) + (3 * DIGIT_SPACING) + COLON_WIDTH + (2 * COLON_SPACING))
#define CLOCK_START_X ((PANEL_WIDTH - TOTAL_CLOCK_WIDTH) / 2)
#define CLOCK_START_Y ((PANEL_HEIGHT - DIGIT_HEIGHT) / 2)

void drawDigit(int x, int y, int digit) {
  // Standard 7-segment patterns (more accurate to real displays)
  bool segments[10][7] = {
    {1,1,1,1,1,1,0}, // 0: A,B,C,D,E,F
    {0,1,1,0,0,0,0}, // 1: B,C only
    {1,1,0,1,1,0,1}, // 2: A,B,D,E,G
    {1,1,1,1,0,0,1}, // 3: A,B,C,D,G
    {0,1,1,0,0,1,1}, // 4: B,C,F,G
    {1,0,1,1,0,1,1}, // 5: A,C,D,F,G
    {1,0,1,1,1,1,1}, // 6: A,C,D,E,F,G
    {1,1,1,0,0,0,0}, // 7: A,B,C only
    {1,1,1,1,1,1,1}, // 8: All segments
    {1,1,1,1,0,1,1}  // 9: A,B,C,D,F,G
  };

  if (digit < 0 || digit > 9) return;
  
  bool* seg = segments[digit];
  
  // Draw segments with professional proportions
  // Segment positions calculated for optimal readability
  
  // Segment A (top horizontal)
  if (seg[0]) matrix->fillRect(x + SEGMENT_GAP, y, 
                               DIGIT_WIDTH - 2*SEGMENT_GAP, SEGMENT_THICK, COLOR_ON);
  
  // Segment B (top right vertical)
  if (seg[1]) matrix->fillRect(x + DIGIT_WIDTH - SEGMENT_THICK, y + SEGMENT_GAP, 
                               SEGMENT_THICK, (DIGIT_HEIGHT/2) - SEGMENT_GAP, COLOR_ON);
  
  // Segment C (bottom right vertical)
  if (seg[2]) matrix->fillRect(x + DIGIT_WIDTH - SEGMENT_THICK, y + (DIGIT_HEIGHT/2) + SEGMENT_GAP, 
                               SEGMENT_THICK, (DIGIT_HEIGHT/2) - SEGMENT_GAP, COLOR_ON);
  
  // Segment D (bottom horizontal)
  if (seg[3]) matrix->fillRect(x + SEGMENT_GAP, y + DIGIT_HEIGHT - SEGMENT_THICK, 
                               DIGIT_WIDTH - 2*SEGMENT_GAP, SEGMENT_THICK, COLOR_ON);
  
  // Segment E (bottom left vertical)
  if (seg[4]) matrix->fillRect(x, y + (DIGIT_HEIGHT/2) + SEGMENT_GAP, 
                               SEGMENT_THICK, (DIGIT_HEIGHT/2) - SEGMENT_GAP, COLOR_ON);
  
  // Segment F (top left vertical)
  if (seg[5]) matrix->fillRect(x, y + SEGMENT_GAP, 
                               SEGMENT_THICK, (DIGIT_HEIGHT/2) - SEGMENT_GAP, COLOR_ON);
  
  // Segment G (middle horizontal)
  if (seg[6]) matrix->fillRect(x + SEGMENT_GAP, y + (DIGIT_HEIGHT/2) - (SEGMENT_THICK/2), 
                               DIGIT_WIDTH - 2*SEGMENT_GAP, SEGMENT_THICK, COLOR_ON);
}

void drawColon(int x, int y) {
  // Professional colon positioning - aligned with segment centers
  int dot_size = SEGMENT_THICK;
  int upper_y = y + (DIGIT_HEIGHT/4) - (dot_size/2);
  int lower_y = y + (3*DIGIT_HEIGHT/4) - (dot_size/2);
  
  matrix->fillRect(x, upper_y, dot_size, dot_size, COLOR_COLON);
  matrix->fillRect(x, lower_y, dot_size, dot_size, COLOR_COLON);
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
  matrix->fillScreenRGB888(0, 0, 0);  // Clear screen

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  // Convert to 12-hour format for standard clock appearance
  int display_hour = hour;
  if (display_hour == 0) display_hour = 12;
  else if (display_hour > 12) display_hour -= 12;

  // Calculate precise positions using professional spacing
  int h1_x = CLOCK_START_X;                                    // Hour tens digit
  int h2_x = h1_x + DIGIT_WIDTH + DIGIT_SPACING;              // Hour units digit
  int colon_x = h2_x + DIGIT_WIDTH + COLON_SPACING;           // Colon
  int m1_x = colon_x + COLON_WIDTH + COLON_SPACING;           // Minute tens digit
  int m2_x = m1_x + DIGIT_WIDTH + DIGIT_SPACING;              // Minute units digit

  // Draw the clock with professional alignment
  // Only show hour tens digit if hour >= 10 (standard clock behavior)
  if (display_hour >= 10) {
    drawDigit(h1_x, CLOCK_START_Y, display_hour / 10);
  }
  drawDigit(h2_x, CLOCK_START_Y, display_hour % 10);
  drawColon(colon_x, CLOCK_START_Y);
  drawDigit(m1_x, CLOCK_START_Y, minute / 10);
  drawDigit(m2_x, CLOCK_START_Y, minute % 10);

  delay(500); // Update every half second
}
