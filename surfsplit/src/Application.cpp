#include <cstdlib>
#include <cstdio>
#include <map>
#include <vector>

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

void Error()
{
    fprintf( stderr, "Usage: surfsplit filename.png [option]\n\n" );
    fprintf( stderr, "Options:\n" );
    fprintf( stderr, " -v     view data layout\n" );
    fprintf( stderr, " -b     set block size (default: 8)\n" );
    exit( 1 );
}

bool viewData = false;
int blockSize = 8;

int main( int argc, char** argv )
{
#define CSTR(x) strcmp( argv[i], x ) == 0

    if( argc < 2 )
    {
        Error();
    }

    for( int i=2; i<argc; i++ )
    {
        if( CSTR( "-v" ) )
        {
            viewData = true;
        }
        else if( CSTR( "-b" ) )
        {
            blockSize = atoi( argv[i+1] );
            i++;
        }
        else
        {
            Error();
        }
    }

#undef CSTR

    std::string out( argv[1] );
    out += ".csr";

    Bitmap bmp( argv[1] );
    if( !bmp.Data() )
    {
        std::vector<Rect> r;
        r.push_back( Rect( 0, 0, bmp.Size().x, bmp.Size().y ) );
        //printf( "Bitmap %s no alpha channel\n", argv[1] );
        Save( out.c_str(), r );
        return 0;
    }

    std::vector<Rect> r( GenerateGrid( bmp.Size(), blockSize, blockSize ) );
    r = RemoveEmpty( r, &bmp );

    std::vector<float> hist( CalcBroadDuplicates( r, &bmp ) );
    std::map<float, std::vector<int> > map;
    for( int i=0; i<hist.size(); i++ )
    {
        map[hist[i]].push_back( i );
    }

    std::vector<Rect> rects;
    std::vector<DupRect> dupes;
    for( auto it = begin( map ); it != end( map ); ++it )
    {
        if( it->second.size() == 1 )
        {
            rects.push_back( r[*it->second.begin()] );
        }
        else
        {
            for( auto lit = begin( it->second ); lit != end( it->second ); ++lit )
            {
                auto cit = lit;
                ++cit;
                std::vector<Rect> d;
                while( cit != end( it->second ) )
                {
                    if( AreExactDuplicates( r[*lit], r[*cit], &bmp ) )
                    {
                        d.push_back( r[*cit] );
                        cit = it->second.erase( cit );
                    }
                    else
                    {
                        ++cit;
                    }
                }
                if( d.empty() )
                {
                    rects.push_back( r[*lit] );
                }
                else
                {
                    Rect& base = r[*lit];
                    DupRect dr( base );
                    dr.n = d.size();
                    dr.xy = new uint16[dr.n * 2];

                    uint16* ptr = dr.xy;
                    for( int i=0; i<dr.n; i++ )
                    {
                        *ptr++ = d[i].x;
                        *ptr++ = d[i].y;
                    }

                    dupes.push_back( dr );
                }
            }
        }
    }

    std::vector<Rect> r1 = MergeHorizontal( r );
    r1 = MergeVertical( r1 );
    r = MergeVertical( r );
    r = MergeHorizontal( r );
    //printf( "H1: %i, V1: %i\n", r1.size(), r.size() );
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
    //printf( "Reduction: %i -> %i (%.2f%%)\n", area, rarea, 100.f * rarea / area );
    Save( out.c_str(), r );

    if( viewData )
    {
        ShowBitmap( &bmp, r );
    }

    return 0;
}
