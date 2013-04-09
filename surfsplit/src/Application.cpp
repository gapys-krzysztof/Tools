#include <cstdlib>
#include <cstdio>
#include <iterator>
#include <map>
#include <vector>

#include "Bitmap.hpp"
#include "Generators.hpp"
#include "Process.hpp"
#include "Video.hpp"

void Save( const char* fn, const std::vector<Rect>& rects, const std::vector<DupRect>& dupes )
{
    FILE* f = fopen( fn, "wb" );

    uint32 s = rects.size();
    fwrite( &s, 1, 4, f );
    fwrite( &*rects.begin(), 1, sizeof( Rect ) * s, f );

    s = dupes.size();
    fwrite( &s, 1, 4, f );
    for( auto it = begin( dupes ); it != end( dupes ); ++it )
    {
        fwrite( &*it, 1, sizeof( uint16 ) * 4, f );
        s = it->xy.size();
        fwrite( &s, 1, 4, f );
        for( auto oit = begin( it->xy ); oit != end( it->xy ); ++oit )
        {
            fwrite( &*oit, 1, sizeof( OffRect ), f );
        }
    }

    fclose( f );
}

void Error()
{
    fprintf( stderr, "Usage: surfsplit filename.png [option]\n\n" );
    fprintf( stderr, "Options:\n" );
    fprintf( stderr, " -v     view data layout\n" );
    fprintf( stderr, " -b     set block size (default: 8)\n" );
    fprintf( stderr, " -d     search for duplicates\n" );
    fprintf( stderr, " -a     set minimum alpha cutoff threshold (default: 0)\n" );
    exit( 1 );
}

bool viewData = false;
int blockSize = 8;
bool searchDuplicates = false;
int alphaCutoff = 0;

void Process( const char* in )
{
    std::string out( in );
    out += ".csr";

    Bitmap bmp( in );

    std::vector<Rect> r( GenerateGrid( bmp.Size(), blockSize, blockSize ) );
    r = RemoveEmpty( r, &bmp );

    std::vector<DupRect> dupes;
    if( searchDuplicates )
    {
        std::vector<int> hist( CalcBroadDuplicates( r, &bmp ) );
        std::map<int, std::vector<int> > map;
        for( int i=0; i<hist.size(); i++ )
        {
            map[hist[i]].push_back( i );
        }

        std::vector<Rect> rects;
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

                        dr.xy.push_back( OffRect( base.x, base.y, base.w, base.h, 0, 0 ) );

                        for( int i=0; i<d.size(); i++ )
                        {
                            dr.xy.push_back( OffRect( d[i].x, d[i].y, d[i].w, d[i].h, 0, 0 ) );
                        }

                        dupes.push_back( dr );
                    }
                }
            }
        }

        r = rects;

        std::vector<DupRect> mdr = Merge( dupes );
        for( auto it = begin( mdr ); it != end( mdr ); ++it )
        {
            it->xy.clear();

            for( auto dit = begin( dupes ); dit != end( dupes ); ++dit )
            {
                if( dit->x >= it->x && dit->x + dit->w <= it->x + it->w &&
                    dit->y >= it->y && dit->y + dit->h <= it->y + it->h )
                {
                    for( auto oit = begin( dit->xy ); oit != end( dit->xy ); ++oit )
                    {
                        it->xy.push_back( OffRect( oit->x, oit->y, oit->w, oit->h, dit->x - it->x, dit->y - it->y ) );
                    }
                }
            }
        }

        for( auto it = begin( mdr ); it != end( mdr ); ++it )
        {
            std::map<std::pair<int, int>, std::vector<OffRect>> m;
            for( auto oit = begin( it->xy ); oit != end( it->xy ); ++oit )
            {
                int bx = ((int)oit->x - (int)it->x)/ (int)it->w;
                int by = ((int)oit->y - (int)it->y) / (int)it->h;
                m[std::make_pair( bx, by )].push_back( *oit );
            }
            it->xy.clear();
            for( auto mit = begin( m ); mit != end( m ); ++mit )
            {
                auto mo = Merge( mit->second );
                std::copy( begin( mo ), end( mo ), std::back_inserter( it->xy ) );
            }
        }

        dupes = mdr;
    }

    r = Merge( r );
    r = CropEmpty( r, &bmp );

    Save( out.c_str(), r, dupes );

    if( viewData )
    {
        ShowBitmap( &bmp, r, dupes );
    }
}

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
            i++;
            blockSize = atoi( argv[i] );
        }
        else if( CSTR( "-d" ) )
        {
            searchDuplicates = true;
        }
        else if( CSTR( "-a" ) )
        {
            i++;
            alphaCutoff = atoi( argv[i] );
        }
        else
        {
            Error();
        }
    }

#undef CSTR

    Process( argv[1] );

    return 0;
}
