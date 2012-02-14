#ifndef __RECT_HPP__
#define __RECT_HPP__

#include "Types.hpp"

struct Rect
{
    Rect( uint16 _x, uint16 _y, uint16 _w, uint16 _h, Bitmap* _b ) : x( _x ), y( _y ), w( _w ), h( _h ), b( _b ) {}

    uint16 x, y, w, h;
    Bitmap* b;
};

#endif
