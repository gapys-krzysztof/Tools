#ifndef __RECT_HPP__
#define __RECT_HPP__

#include <string>

#include "Types.hpp"

class Bitmap;

struct Rect
{
    Rect( uint16 _x, uint16 _y, uint16 _w, uint16 _h ) : x( _x ), y( _y ), w( _w ), h( _h ) {}

    uint16 x, y, w, h;
};

struct BRect
{
    BRect( uint16 _x, uint16 _y, uint16 _w, uint16 _h, Bitmap* _b, const std::string& _name ) : x( _x ), y( _y ), w( _w ), h( _h ), b( _b ), name( _name ) {}

    uint16 x, y, w, h;
    Bitmap* b;
    std::string name;
};

#endif
