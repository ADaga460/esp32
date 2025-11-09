#include "display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pins used in your starter code. Change if wiring differs.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 9
#define OLED_CS 10
#define OLED_RESET 8

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                                &SPI, OLED_DC, OLED_RESET, OLED_CS);

void display_init()
{
    Serial.println("Init OLED...");
    if (!display.begin(SSD1306_SWITCHCAPVCC))
    {
        Serial.println("SSD1306 allocation failed!");
        for (;;)
            ;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.display();
    delay(50);
}

void display_render_text(const char *text)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    // Simple wrapping logic for 128x64, size 1 -> approx 21 chars per line
    const int charsPerLine = 20; // conservative
    String s = String(text);
    int idx = 0;
    int line = 0;
    while (idx < s.length() && line < 6)
    { // up to 6 lines
        String chunk = s.substring(idx, min(idx + charsPerLine, (int)s.length()));
        display.setCursor(0, line * 10); // 10 px line height approximate
        display.println(chunk);
        idx += charsPerLine;
        line++;
    }
    display.display();
}
