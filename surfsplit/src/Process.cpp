#include <list>

#include "Process.hpp"

enum { RedMask   = 0x000000FF };
enum { GreenMask = 0x0000FF00 };
enum { BlueMask  = 0x00FF0000 };
enum { AlphaMask = 0xFF000000 };

enum { RedShift   = 0 };
enum { GreenShift = 8 };
enum { BlueShift  = 16 };
enum { AlphaShift = 24 };

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
            if( ( *ptr++ & AlphaMask ) != 0 )
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
            if( ( *ptr++ & AlphaMask ) != 0 )
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
            if( ( *ptr++ & AlphaMask ) != 0 )
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
            if( ( *ptr & AlphaMask ) != 0 )
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
            if( ( *ptr & AlphaMask ) != 0 )
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

int CalcHistogram( const Rect& rect, Bitmap* bmp )
{
    int bw = bmp->Size().x;
    uint32* ptr = bmp->Data() + rect.x + rect.y * bw;
    int h = rect.h;

    int hist = 0;

    while( h-- )
    {
        int w = rect.w;

        while( w-- )
        {
            if( ( *ptr & AlphaMask ) != 0 )
            {
                int r = ( ( *ptr & RedMask   ) >> RedShift   );
                int g = ( ( *ptr & GreenMask ) >> GreenShift );
                int b = ( ( *ptr & BlueMask  ) >> BlueShift  );

                hist += ( r * 77 + g * 151 + b * 28 ) >> 8;
            }
            ptr++;
        }

        ptr += bw - rect.w;
    }

    return hist;
}

std::vector<int> CalcBroadDuplicates( const std::vector<Rect>& rects, Bitmap* bmp )
{
    std::vector<int> ret;
    ret.reserve( rects.size() );

    for( std::vector<Rect>::const_iterator it = rects.begin(); it != rects.end(); ++it )
    {
        ret.push_back( CalcHistogram( *it, bmp ) );
    }

    return ret;
}

bool AreExactDuplicates( const Rect& r1, const Rect& r2, Bitmap* bmp )
{
    if( r1.w != r2.w || r1.h != r2.h ) return false;

    int bw = bmp->Size().x;
    uint32* ptr1 = bmp->Data() + r1.x + r1.y * bw;
    uint32* ptr2 = bmp->Data() + r2.x + r2.y * bw;
    int h = r1.h;

    while( h-- )
    {
        int w = r1.w;

        while( w-- )
        {
            uint32 c1 = *ptr1++;
            uint32 c2 = *ptr2++;

            if( ( ( c1 | c2 ) & AlphaMask ) != 0 && c1 != c2 )
            {
                return false;
            }
        }

        ptr1 += bw - r1.w;
        ptr2 += bw - r1.w;
    }

    return true;
}
