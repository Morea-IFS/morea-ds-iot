// Essential libs
#include "Arduino.h"
#include "Layout.h"

// Display libs
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int displayIconsXAxis[] = {3, 13, 23, 33, 89, 99, 109, 119};
int displayTextYAxis[] = {16, 26, 36, 51};

Layout::Layout(int screen_width, int screen_height, int reset_pin) : display(screen_width, screen_height, &Wire, reset_pin)
{
    // Display Initialization

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Try to Detect and Inicialize Display
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
}

void Layout::beginLayout()
{
    // Drawing Basic Layout on Display
    display.clearDisplay();

    display.writeFastHLine(0, 13, 128, SSD1306_WHITE);
    display.writePixel(3, 46, SSD1306_WHITE);
    display.writeFastHLine(3, 47, 105, SSD1306_WHITE);
    display.writePixel(3, 48, SSD1306_WHITE);
    display.writePixel(3, 61, SSD1306_WHITE);
    display.writeFastHLine(3, 62, 105, SSD1306_WHITE);

    // Setting text properties
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.display();
}

void Layout::beginTimer()
{
    display.setCursor(54, 3);
    display.println("0:00");

    display.display();
}

void Layout::updateTimer(int counter, byte interval)
{
    // Interval correspond to the time passed beetween every counter
    //  Calculating the Timer Values
    int minutes = 4 - floor((counter * interval) / 60);
    int seconds = 60 - ((counter * interval) % 60);

    if (seconds == 60)
    {
        minutes = minutes + 1;
        seconds = 0;
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.writeFillRect(54, 3, 25, 8, SSD1306_BLACK);
    display.setCursor(54, 3);

    if (seconds < 10)
    {
        display.println(String(minutes) + ":0" + String(seconds));
    }
    else
    {
        display.println(String(minutes) + ":" + String(seconds));
    }

    display.display();
}

void Layout::drawLogo(const unsigned char *logo)
{
    display.drawBitmap(111, 47, logo, 16, 16, 1);
    display.display();
}

void Layout::drawIcon(int index, const unsigned char *logo)
{
    display.writeFillRect(displayIconsXAxis[index], 3, 8, 8, SSD1306_BLACK);
    display.drawBitmap(displayIconsXAxis[index], 3, logo, 8, 8, 1);

    display.display();
}

void Layout::eraseIcon(int index)
{
    display.writeFillRect(displayIconsXAxis[index], 3, 8, 8, SSD1306_BLACK);

    display.display();
}

void Layout::writeLine(int index, String text)
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.writeFillRect(3, displayTextYAxis[index], 124, 8, SSD1306_BLACK);
    display.setCursor(3, displayTextYAxis[index]);

    display.println(text);

    display.display();
}