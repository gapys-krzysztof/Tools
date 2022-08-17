#ifndef __RECT_HPP__
#define __RECT_HPP__

#include <stdint.h>
#include <string>

class Bitmap;

struct Rect
{
    Rect( uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h ) : x( _x ), y( _y ), w( _w ), h( _h ) {}

    uint16_t x, y, w, h;
};

struct BRect
{
    BRect() = default;
    BRect( uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, Bitmap* _b, const std::string& _name, int maxRect = 0 ) : x( _x ), y( _y ), w( _w ), h( _h ), b( _b ), name( _name ), flip( false ), maxRectSize( maxRect ) {}

    uint16_t x, y, w, h;
    Bitmap* b;
    std::string name;
    bool flip;
    int maxRectSize; // maximum rect size for current asset
};

#endif
