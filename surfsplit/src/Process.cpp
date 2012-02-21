#include <list>

#include "Process.hpp"

bool IsEmpty( const Rect& rect, Bitmap* bmp )
{
    int bw = bmp->Size().x;
    uint32* ptr = bmp->Data() + rect.x + rect.y * bw;
    int h = rect.h;

    while( h-- )
    {
        int w = rect.w;

        while( w-- )
        {
            if( ( *ptr++ & 0xFF000000 ) != 0 )
            {
                return false;
            }
        }

        ptr += bw - rect.w;
    }

    return true;
}

std::vector<Rect> RemoveEmpty( const std::vector<Rect>& rects, Bitmap* bmp )
{
    std::vector<Rect> ret;
    ret.reserve( rects.size() );

    for( std::vector<Rect>::const_iterator it = rects.begin(); it != rects.end(); ++it )
    {
        if( !IsEmpty( *it, bmp ) )
        {
            ret.push_back( *it );
        }
    }

    //printf( "Remove empty rects: %i -> %i\n", rects.size(), ret.size() );

    return ret;
}

Rect CropEmpty( const Rect& rect, Bitmap* bmp )
{
    Rect ret( rect );

    int bw = bmp->Size().x;
    uint32* ptr = bmp->Data() + ret.x + ret.y * bw;

    int h = ret.h;
    for( int i=0; i<h; i++ )
    {
        int w = ret.w;
        while( w-- )
        {
            if( ( *ptr++ & 0xFF000000 ) != 0 )
            {
                goto next1;
            }
        }
        ret.y++;
        ret.h--;

        ptr += bw - ret.w;
    }

next1:
    ptr = bmp->Data() + ret.x + ( ret.y + ret.h - 1 ) * bw;
    h = ret.h;
    for( int i=0; i<h; i++ )
    {
        int w = ret.w;
        while( w-- )
        {
            if( ( *ptr++ & 0xFF000000 ) != 0 )
            {
                goto next2;
            }
        }
        ret.h--;
        ptr -= bw + ret.w;
    }

next2:
    ptr = bmp->Data() + ret.x + ret.y * bw;
    int w = ret.w;
    for( int i=0; i<w; i++ )
    {
        h = ret.h;
        while( h-- )
        {
            if( ( *ptr & 0xFF000000 ) != 0 )
            {
                goto next3;
            }
            ptr += bw;
        }
        ret.x++;
        ret.w--;
        ptr -= bw * ret.h - 1;
    }

next3:
    ptr = bmp->Data() + ret.x + ret.w - 1 + ret.y * bw;
    w = ret.w;
    for( int i=0; i<w; i++ )
    {
        h = ret.h;
        while( h-- )
        {
            if( ( *ptr & 0xFF000000 ) != 0 )
            {
                goto next4;
            }
            ptr += bw;
        }
        ret.w--;
        ptr -= bw * ret.h + 1;
    }

next4:
    return ret;
}

std::vector<Rect> CropEmpty( const std::vector<Rect>& rects, Bitmap* bmp )
{
    std::vector<Rect> ret;
    ret.reserve( rects.size() );

    for( std::vector<Rect>::const_iterator it = rects.begin(); it != rects.end(); ++it )
    {
        ret.push_back( CropEmpty( *it, bmp ) );
    }

    return ret;
}

std::vector<Rect> MergeHorizontal( const std::vector<Rect>& rects )
{
    std::list<Rect> tmp( rects.begin(), rects.end() );

    std::list<Rect>::iterator it = tmp.begin();
    while( it != tmp.end() )
    {
        bool merged = false;
        for( std::list<Rect>::iterator tit = tmp.begin(); tit != tmp.end(); tit++ )
        {
            if( it != tit && it->y == tit->y && it->h == tit->h )
            {
                if( it->x + it->w == tit->x )
                {
                    it->w += tit->w;
                    tmp.erase( tit );
                    merged = true;
                    break;
                }
                else if( it->x == tit->x + tit->w )
                {
                    it->x -= tit->w;
                    tmp.erase( tit );
                    merged = true;
                    break;
                }
            }
        }
        if( !merged )
        {
            it++;
        }
    }

    std::vector<Rect> ret( tmp.begin(), tmp.end() );
    //printf( "Merge horizontal: %i -> %i\n", rects.size(), ret.size() );
    return ret;
}

std::vector<Rect> MergeVertical( const std::vector<Rect>& rects )
{
    std::list<Rect> tmp( rects.begin(), rects.end() );

    std::list<Rect>::iterator it = tmp.begin();
    while( it != tmp.end() )
    {
        bool merged = false;
        for( std::list<Rect>::iterator tit = tmp.begin(); tit != tmp.end(); tit++ )
        {
            if( it != tit && it->x == tit->x && it->w == tit->w )
            {
                if( it->y + it->h == tit->y )
                {
                    it->h += tit->h;
                    tmp.erase( tit );
                    merged = true;
                    break;
                }
                else if( it->y == tit->y + tit->h )
                {
                    it->y -= tit->h;
                    tmp.erase( tit );
                    merged = true;
                    break;
                }
            }
        }
        if( !merged )
        {
            it++;
        }
    }

    std::vector<Rect> ret( tmp.begin(), tmp.end() );
    //printf( "Merge vertical: %i -> %i\n", rects.size(), ret.size() );
    return ret;
}
