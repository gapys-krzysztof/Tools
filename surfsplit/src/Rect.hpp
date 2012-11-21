#ifndef __RECT_HPP__
#define __RECT_HPP__

#include "Types.hpp"

struct Rect
{
    Rect( uint16 _x, uint16 _y, uint16 _w, uint16 _h ) : x( _x ), y( _y ), w( _w ), h( _h ) {}

    uint16 x, y, w, h;
};

struct DupRect
{
    DupRect( const Rect& r ) : x( r.x ), y( r.y ), w( r.w ), h( r.h ) {}

    uint16* xy;
    uint16 x, y;
    uint16 w, h;
    uint16 n;
};

#endif
