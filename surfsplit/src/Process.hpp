#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <algorithm>
#include <vector>

#include "Bitmap.hpp"
#include "Rect.hpp"

bool IsEmpty( const Rect& rect, Bitmap* bmp );
std::vector<Rect> RemoveEmpty( const std::vector<Rect>& rects, Bitmap* bmp );

Rect CropEmpty( const Rect& rect, Bitmap* bmp );
std::vector<Rect> CropEmpty( const std::vector<Rect>& rects, Bitmap* bmp );

template<typename T>
std::vector<T> MergeHorizontal( const std::vector<T>& rects )
{
    std::vector<T> ret( rects );
    std::sort( begin( ret ), end( ret ), []( const Rect& r1, const Rect& r2 ){ return r1.y == r2.y ? r1.x < r2.x : r1.y < r2.y; } );

    for( auto it = begin( ret ); it != end( ret ); ++it )
    {
        auto tit = it;
        ++tit;

        int tx = it->x + it->w;

        while( tit != end( ret ) && tit->y == it->y && tit->x == tx && tit->h == it->h )
        {
            it->w += tit->w;
            tx += tit->w;
            tit = ret.erase( tit );
        }
    }

    return ret;
}

template<typename T>
std::vector<T> MergeVertical( const std::vector<T>& rects )
{
    std::vector<T> ret( rects );
    std::sort( begin( ret ), end( ret ), []( const Rect& r1, const Rect& r2 ){ return r1.x == r2.x ? r1.y < r2.y : r1.x < r2.x; } );

    for( auto it = begin( ret ); it != end( ret ); ++it )
    {
        auto tit = it;
        ++tit;

        int ty = it->y + it->h;

        while( tit != end( ret ) && tit->x == it->x && tit->y == ty && tit->w == it->w )
        {
            it->h += tit->h;
            ty += tit->h;
            tit = ret.erase( tit );
        }
    }

    return ret;
}

std::vector<int> CalcBroadDuplicates( const std::vector<Rect>& rects, Bitmap* bmp );
bool AreExactDuplicates( const Rect& rect1, const Rect& rect2, Bitmap* bmp );

#endif
