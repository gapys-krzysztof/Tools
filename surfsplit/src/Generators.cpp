#include <algorithm>

#include "Generators.hpp"

std::vector<Rect> GenerateGrid( const v2i& size, int x, int y )
{
    std::vector<Rect> ret;
    ret.reserve( ( size.x + x ) / x * ( size.y + y ) / y );

    for( int j=0; j<size.y; j+=y )
    {
        for( int i=0; i<size.x; i+=x )
        {
            ret.push_back( Rect( i, j, std::min( x, size.x - i ), std::min( y, size.y - j ) ) );
        }
    }

    printf( "Generate %ix%i grid -> %i rects\n", x, y, ret.size() );

    return ret;
}
