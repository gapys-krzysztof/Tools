#ifndef __RECT_HPP__
#define __RECT_HPP__

#include <string>
#include <vector>

#include "Types.hpp"

class Bitmap;

struct Rect
{
    Rect( uint16 _x, uint16 _y, uint16 _w, uint16 _h ) : x( _x ), y( _y ), w( _w ), h( _h ) {}

    uint16 x, y, w, h;
};

struct OffRect
{
    uint16 x, y, w, h, ox, oy;
};

struct BRect
{
    BRect( uint16 _x, uint16 _y, uint16 _w, uint16 _h, Bitmap* _b, const std::string& _name ) : x( _x ), y( _y ), w( _w ), h( _h ), b( _b ), name( _name ), flip( false ) {}
    BRect( const BRect& r )
        : x( r.x )
        , y( r.y )
        , w( r.w )
        , h( r.h )
        , b( r.b )
        , name( r.name )
        , flip( r.flip )
        , xy( r.xy )
    {
    }

    BRect& operator=( const BRect& r )
    {
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;
        b = r.b;
        name = r.name;
        flip = r.flip;
        xy = r.xy;
        return *this;
    }

    uint16 x, y, w, h;
    Bitmap* b;
    std::string name;
    bool flip;
    std::vector<OffRect> xy;
};

#endif
