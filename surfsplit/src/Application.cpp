#include <cstdlib>
#include <cstdio>

#include "Bitmap.hpp"
#include "Generators.hpp"
#include "Process.hpp"
#include "Video.hpp"

void Save( const char* fn, const std::vector<Rect>& rects )
{
    FILE* f = fopen( fn, "wb" );
    uint32 s = rects.size();
    fwrite( &s, 1, 4, f );
    fwrite( &*rects.begin(), 1, sizeof( Rect ) * s, f );
    fclose( f );
}

int main( int argc, char** argv )
{
    if( argc != 2 )
    {
        fprintf( stderr, "No input file name given.\n" );
        exit( 1 );
    }

    std::string out( argv[1] );
    out += ".csr";

    Bitmap bmp( argv[1] );
    if( !bmp.Data() )
    {
        std::vector<Rect> r;
        r.push_back( Rect( 0, 0, bmp.Size().x, bmp.Size().y ) );
        printf( "Bitmap %s no alpha channel\n", argv[1] );
        Save( out.c_str(), r );
        return 0;
    }

    std::vector<Rect> r( GenerateGrid( bmp.Size(), 8, 8 ) );
    r = RemoveEmpty( r, &bmp );
    std::vector<Rect> r1 = MergeHorizontal( r );
    r1 = MergeVertical( r1 );
    r = MergeVertical( r );
    r = MergeHorizontal( r );
    printf( "H1: %i, V1: %i\n", r1.size(), r.size() );
    if( r1.size() < r.size() )
    {
        r = r1;
    }
    r = CropEmpty( r, &bmp );
    int area = bmp.Size().x * bmp.Size().y;
    int rarea = 0;
    for( std::vector<Rect>::const_iterator it = r.begin(); it != r.end(); ++it )
    {
        rarea += it->w * it->h;
    }
    printf( "Reduction: %i -> %i (%.2f%%)\n", area, rarea, 100.f * rarea / area );
    Save( out.c_str(), r );
    //ShowBitmap( &bmp, r );

    return 0;
}
