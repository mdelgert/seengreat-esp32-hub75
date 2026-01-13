#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 1

// Correct pin mapping for ESP32-S3 + Matrix Adapter Board (E)
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
    .e  = -1,     // << IMPORTANT: 1/16 scan panel → NO E pin

    .lat = 38,
    .oe  = 21,
    .clk = 47
};

HUB75_I2S_CFG mxconfig(
    PANEL_WIDTH,
    PANEL_HEIGHT,
    PANELS_NUMBER,
    pins
);

MatrixPanel_I2S_DMA *matrix = nullptr;

void setup() {
    Serial.begin(115200);

    matrix = new MatrixPanel_I2S_DMA(mxconfig);
    matrix->setBrightness8(180);

    if (!matrix->begin()) {
        Serial.println("DMA init failed!");
        while (true);
    }

    matrix->fillScreenRGB888(0, 0, 0); // clear

    matrix->setTextSize(1);
    matrix->setTextColor(matrix->color565(255, 255, 0)); // yellow
    matrix->setCursor(4, 12); // good vertical alignment for 32px tall panel
    matrix->print("HELLO");
}

void loop() {
    // static – nothing here
}
