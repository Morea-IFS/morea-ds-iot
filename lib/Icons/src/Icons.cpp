#include "Arduino.h"
#include "Icons.h"

const unsigned char Icons::_sparcLogo[] PROGMEM =
    {B00000000, B10000000,
     B00000000, B10000000,
     B00000001, B01000000,
     B00000001, B01000000,
     B00000001, B01000000,
     B00010010, B00100000,
     B00111010, B00100000,
     B00010100, B10010000,
     B00100100, B11010000,
     B00101000, B10001000,
     B00101001, B00001000,
     B00101001, B00001000,
     B00100100, B00010000,
     B00010011, B10010000,
     B00001000, B00100000,
     B00000111, B11000000};

const unsigned char Icons::_loadingIcon[] PROGMEM = {
    B00001000,
    B01100100,
    B00100010,
    B10000001,
    B10000001,
    B01000100,
    B00100110,
    B00010000};

const unsigned char Icons::_failedIcon[] PROGMEM = {
    B10000010,
    B01000100,
    B00101000,
    B00010000,
    B00101000,
    B01000100,
    B10000010,
    B00000000};

const unsigned char Icons::_successIcon[] PROGMEM = {
    B00000000,
    B00000001,
    B00000010,
    B00000100,
    B10001000,
    B01010000,
    B00100000,
    B00000000};

const unsigned char Icons::_keyIcon[] PROGMEM = {
    B00111000,
    B00011000,
    B00011000,
    B00111100,
    B01100110,
    B01000010,
    B01100110,
    B00111100};

const unsigned char Icons::_wifiIcon[] PROGMEM = {
    B00111000,
    B01000100,
    B10000010,
    B00111000,
    B01000100,
    B00000000,
    B00010000,
    B00000000};

const unsigned char Icons::_waterIcon[] PROGMEM = {
    B00011000,
    B00011000,
    B00111100,
    B00111100,
    B01111110,
    B01111110,
    B00111100,
    B00011000};

const unsigned char Icons::_wrenchIcon[] PROGMEM = {
    B00100100,
    B01100110,
    B01100110,
    B00111100,
    B00011000,
    B00011000,
    B00011000,
    B00011000};

Icons::Icons() {}

const unsigned char *Icons::sparcLogo()
{
    return _sparcLogo;
}

const unsigned char *Icons::loadingIcon()
{
    return _loadingIcon;
}

const unsigned char *Icons::failedIcon()
{
    return _failedIcon;
}

const unsigned char *Icons::successIcon()
{
    return _successIcon;
}

const unsigned char *Icons::keyIcon()
{
    return _keyIcon;
}

const unsigned char *Icons::wifiIcon()
{
    return _wifiIcon;
}

const unsigned char *Icons::waterIcon()
{
    return _waterIcon;
}

const unsigned char *Icons::wrenchIcon()
{
    return _wrenchIcon;
}