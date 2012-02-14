#ifndef __DARKRL__BITMAP_HPP__
#define __DARKRL__BITMAP_HPP__

#include "Rect.hpp"
#include "Types.hpp"
#include "Vector.hpp"

class Bitmap
{
public:
    Bitmap( int x, int y );
    Bitmap( const Bitmap& bmp );
    Bitmap( const char* fn );
    ~Bitmap();

    bool Write( const char* fn, bool alpha = true );

    uint32* Data() const { return m_data; }
    const v2i& Size() const { return m_size; }

    Bitmap& operator=( const Bitmap& bmp );

private:
    uint32* m_data;
    v2i m_size;
};

void Blit( Bitmap* dst, Bitmap* src, const Rect& rect );

#endif
