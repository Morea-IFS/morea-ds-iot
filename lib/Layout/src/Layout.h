#ifndef Layout_h
#define Layout_h
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Arduino.h"

class Layout
{
public:
    Layout(int screen_width, int screen_height, int reset_pin);
    void beginLayout();
    void beginTimer();
    void updateTimer(int counter, byte interval);
    void drawLogo(const unsigned char *logo);
    void drawIcon(int index, const unsigned char *logo);
    void eraseIcon(int index);
    void writeLine(int index, String text);

private:
    Adafruit_SSD1306 display;
};

#endif