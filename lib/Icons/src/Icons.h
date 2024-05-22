#ifndef Icons_h
#define Icons_h

#include <avr/pgmspace.h> // Include this to use PROGMEM

class Icons
{
public:
    Icons();
    static const unsigned char *sparcLogo();
    static const unsigned char *loadingIcon();
    static const unsigned char *failedIcon();
    static const unsigned char *successIcon();
    static const unsigned char *keyIcon();
    static const unsigned char *wifiIcon();
    static const unsigned char *waterIcon();
    static const unsigned char *wrenchIcon();

private:
    static const unsigned char _sparcLogo[] PROGMEM;
    static const unsigned char _loadingIcon[] PROGMEM;
    static const unsigned char _failedIcon[] PROGMEM;
    static const unsigned char _successIcon[] PROGMEM;
    static const unsigned char _keyIcon[] PROGMEM;
    static const unsigned char _wifiIcon[] PROGMEM;
    static const unsigned char _waterIcon[] PROGMEM;
    static const unsigned char _wrenchIcon[] PROGMEM;
};

#endif