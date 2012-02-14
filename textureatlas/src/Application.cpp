#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <algorithm>

#include "Bitmap.hpp"
#include "Node.hpp"
#include "Rect.hpp"
#include "String.hpp"

std::string input;
std::string output( "." );
int size = 1024;
std::string name( "atlas" );
std::string sortby( "height" );
int edges = 0;
int path = -1;
bool potw = false;
bool poth = false;
bool align = false;
std::string prepend;
bool square = false;
bool noalpha = false;


std::vector<BRect> LoadImages( const std::list<std::string> pngs, const std::list<std::string> names )
{
    std::vector<BRect> ret;

    std::list<std::string>::const_iterator nit = names.begin();

    ret.reserve( pngs.size() );
    for( std::list<std::string>::const_iterator it = pngs.begin(); it != pngs.end(); ++it, ++nit )
    {
        Bitmap* b = new Bitmap( it->c_str() );
        ret.push_back( BRect( 0, 0, b->Size().x, b->Size().y, b, *nit ) );
    }

    return ret;
}

void SortImages( std::vector<BRect>& images )
{
    struct
    {
        bool operator()( const BRect& i1, const BRect& i2 )
        {
            int a1 = i1.w * i1.h;
            int a2 = i2.w * i2.h;
            if( a1 != a2 ) return a1 > a2;
            if( i1.w != i2.w ) return i1.w > i2.w;
            return i1.h > i2.h;
        }
    } AreaComparator;
    struct
    {
        bool operator()( const BRect& i1, const BRect& i2 )
        {
            if( i1.w != i2.w ) return i1.w > i2.w;
            return i1.h > i2.h;
        }
    } WidthComparator;
    struct
    {
        bool operator()( const BRect& i1, const BRect& i2 )
        {
            if( i1.h != i2.h ) return i1.h > i2.h;
            return i1.w > i2.w;
        }
    } HeightComparator;

    if( sortby == "height" )
    {
        std::sort( images.begin(), images.end(), HeightComparator );
    }
    else if( sortby == "width" )
    {
        std::sort( images.begin(), images.end(), WidthComparator );
    }
    else
    {
        std::sort( images.begin(), images.end(), AreaComparator );
    }
}

bool DoWork()
{
    FILE* f = fopen( input.c_str(), "r" );
    if( !f ) return false;

    std::list<std::string> names, pngnames, rectnames;
    std::string line;
    while( ReadLine( f, line ) )
    {
        names.push_back( line );
        pngnames.push_back( line.substr( 0, line.rfind( '.' ) ) + ".png" );
        rectnames.push_back( line + ".csr" );
        line.clear();
    }

    fclose( f );

    std::vector<BRect> images( LoadImages( pngnames, names ) );
    SortImages( images );

    Bitmap* b = new Bitmap( size, size );
    Node* tree = new Node( Rect( 0, 0, size, size ) );

    int maxWidth = 1;
    int maxHeight = 1;

    for( std::vector<BRect>::const_iterator it = images.begin(); it != images.end(); ++it )
    {
        int ow = it->b->Size().x;
        int oh = it->b->Size().y;

        if( edges != 0 ) {}
        if( align ) {}

        Node* uv = tree->Insert( Rect( 0, 0, ow, oh ), align );
        if( !uv )
        {
            delete b;
            delete tree;
            return false;
        }

        Blit( b, *it, uv->rect );
    }

    b->Write( "out.png" );

    delete b;
    delete tree;

    return true;
}

void Usage()
{
    printf( "Usage: TextureAtlas [options]\n\n" );
    printf( "-i, --input        input text file with input files list\n" );
    printf( "-o, --output       output path (default: current dir)\n" );
    printf( "-s, --size         target atlas size (default: 512)\n" );
    printf( "-e, --edges        duplicate image edges (default: 0)\n" );
    printf( "-b, --border       generate border around non-transparent areas\n" );
    printf( "-n, --name         name of generated files (default: atlas)\n" );
    printf( "-S, --sortby       sort type, possible values: area, width, height (default: height)\n" );
    printf( "-w, --show         opens generated atlas image in default editor\n" );
    printf( "-h, --help         prints this message\n" );
    printf( "-P, --path         path strip depth\n" );
    printf( "-W, --potw         make width of atlas a power of two\n" );
    printf( "-H, --poth         make height of atlas a power of two\n" );
    printf( "-a, --align        align textures to 4x4 blocks\n" );
    printf( "-p, --prepend      prepend given string to all asset paths\n" );
    printf( "-q, --square       make width equal to height\n" );
    printf( "-N, --noalpha      no alpha channel\n" );
}

void Error()
{
    Usage();
    exit( 1 );
}

int main( int argc, char** argv )
{
#define CSTR(x) strcmp( argv[i], x ) == 0

    for( int i=1; i<argc; i++ )
    {
        if( CSTR( "-i" ) || CSTR( "--input" ) )
        {
            input = argv[i+1];
            i++;
        }
        else if( CSTR( "-o" ) || CSTR( "--output" ) )
        {
            output = argv[i+1];
            i++;
        }
        else if( CSTR( "-s" ) || CSTR( "--size" ) )
        {
            size = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-e" ) || CSTR( "--edges" ) )
        {
            edges = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-n" ) || CSTR( "--name" ) )
        {
            name = argv[i+1];
            i++;
        }
        else if( CSTR( "-S" ) || CSTR( "--sortby" ) )
        {
            i++;
            if( !( CSTR( "area" ) || CSTR( "width" ) || CSTR( "height" ) ) )
            {
                Error();
            }
            sortby = argv[i];
        }
        else if( CSTR( "-h" ) || CSTR( "--help" ) )
        {
            Usage();
            return 0;
        }
        else if( CSTR( "-P" ) || CSTR( "--path" ) )
        {
            path = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-W" ) || CSTR( "--potw" ) )
        {
            potw = true;
        }
        else if( CSTR( "-H" ) || CSTR( "--poth" ) )
        {
            poth = true;
        }
        else if( CSTR( "-a" ) || CSTR( "--align" ) )
        {
            align = true;
        }
        else if( CSTR( "-p" ) || CSTR( "--prepend" ) )
        {
            prepend = argv[i+1];
            i++;
        }
        else if( CSTR( "-q" ) || CSTR( "--square" ) )
        {
            square = true;
        }
        else if( CSTR( "-N" ) || CSTR( "--noalpha" ) )
        {
            noalpha = true;
        }
        else
        {
            Error();
        }
    }

#undef CSTR

    if( !DoWork() )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
