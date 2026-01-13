#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_GFX.h>


#define R1_PIN  37
#define G1_PIN  6
#define B1_PIN  36
#define R2_PIN  35
#define G2_PIN  5
#define B2_PIN  0
#define A_PIN   45
#define B_PIN   1
#define C_PIN   48
#define D_PIN   2
#define E_PIN   4
#define LAT_PIN 38
#define OE_PIN  21
#define CLK_PIN 47

HUB75_I2S_CFG::i2s_pins pins = {
  .r1 = R1_PIN, .g1 = G1_PIN, .b1 = B1_PIN,
  .r2 = R2_PIN, .g2 = G2_PIN, .b2 = B2_PIN,
  .a = A_PIN, .b = B_PIN, .c = C_PIN, .d = D_PIN, .e = E_PIN,
  .lat = LAT_PIN, .oe = OE_PIN, .clk = CLK_PIN
};

HUB75_I2S_CFG mxconfig(
  64,     
  64,     
  1,      
  pins    
);

MatrixPanel_I2S_DMA* matrix = new MatrixPanel_I2S_DMA(mxconfig);

void gradientEffect(uint8_t r, uint8_t g, uint8_t b, int duration) {
  const int steps = 50;  
  const int delayTime = duration / (2 * steps);  
  
  for (int i = 0; i <= steps; i++) {
    int brightness = map(i, 0, steps, 0, 255);
    matrix->fillScreen(matrix->color565(
      (r * brightness) / 255,
      (g * brightness) / 255,
      (b * brightness) / 255
    ));
    delay(delayTime);
  }
  
  
  for (int i = steps; i >= 0; i--) {
    int brightness = map(i, 0, steps, 0, 255);
    matrix->fillScreen(matrix->color565(
      (r * brightness) / 255,
      (g * brightness) / 255,
      (b * brightness) / 255
    ));
    delay(delayTime);
  }
}





void showText() {
  matrix->setTextSize(2);             
  matrix->setTextWrap(false);         
  matrix->setTextColor(matrix->color565(0, 255, 255));  
  
  
  const char* text = "SG";
  int textWidth = 9 * 12;  
  int textHeight = 16;     
  int x = (64 - textWidth) / 2;
  int y = (64 - textHeight) / 2;
  
  matrix->setCursor(x, y);
  matrix->print(text);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  if (!matrix->begin()) {
    Serial.println("Matrix initialization failed!");
    while (1);
  }
  
  matrix->setBrightness(100);
  matrix->clearScreen();
  Serial.println("Matrix initialization successful!");
}

void loop() {
 

  gradientEffect(255, 0, 0, 2000);    
  gradientEffect(0, 0, 255, 2000);   
  gradientEffect(0, 255, 0, 2000);    
  gradientEffect(255, 255, 255, 2000);
  

  matrix->clearScreen();
  showText();
  delay(2000);
  

  matrix->clearScreen();
}
