#ifndef __RECT_HPP__
#define __RECT_HPP__

#include "Types.hpp"

struct Rect
{
    Rect( uint16 _x, uint16 _y, uint16 _w, uint16 _h ) : x( _x ), y( _y ), w( _w ), h( _h ) {}

    uint16 x, y, w, h;
};

struct OffRect
{
    OffRect() {}
    OffRect( uint16 x, uint16 y, uint16 w, uint16 h, uint16 ox, uint16 oy ) : x( x ), y( y ), w( w ), h( h ), ox( ox ), oy( oy ) {}

    uint16 x, y, w, h, ox, oy;
};

struct DupRect
{
    DupRect( const Rect& r ) : x( r.x ), y( r.y ), w( r.w ), h( r.h ) {}

    OffRect* xy;
    uint16 x, y;
    uint16 w, h;
    uint16 n;
};

#endif
