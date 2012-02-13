#ifndef __DARKRL__BITMAP_HPP__
#define __DARKRL__BITMAP_HPP__

#include "Types.hpp"
#include "Vector.hpp"

class Bitmap
{
public:
    Bitmap( int x, int y );
    Bitmap( const char* fn );
    ~Bitmap();

    uint32* Data() const { return m_data; }
    const v2i& Size() const { return m_size; }

private:
    uint32* m_data;
    v2i m_size;
};

#endif
