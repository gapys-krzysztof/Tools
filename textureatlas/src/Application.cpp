#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <algorithm>

#include "Bitmap.hpp"
#include "String.hpp"

std::string input;
std::string output( "." );
int size = 512;
std::string name( "atlas" );
std::string sortby( "height" );
std::string order( "desc" );
int edges = 0;
int path = -1;
bool potw = false;
bool poth = false;
bool align = false;
std::string prepend;
bool square = false;
bool noalpha = false;


std::vector<Bitmap*> LoadImages( const std::list<std::string> pngs )
{
    std::vector<Bitmap*> ret;

    ret.reserve( pngs.size() );
    for( std::list<std::string>::const_iterator it = pngs.begin(); it != pngs.end(); ++it )
    {
        ret.push_back( new Bitmap( it->c_str() ) );
    }

    return ret;
}

void SortImages( std::vector<Bitmap*>& images )
{
    struct
    {
        bool operator()( Bitmap* i1, Bitmap* i2 ) { return i1->Size().x * i1->Size().y > i2->Size().x * i2->Size().y; }
    } AreaComparator;
    struct
    {
        bool operator()( Bitmap* i1, Bitmap* i2 ) { return i1->Size().x > i2->Size().x; }
    } WidthComparator;
    struct
    {
        bool operator()( Bitmap* i1, Bitmap* i2 ) { return i1->Size().y > i2->Size().y; }
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

    std::vector<Bitmap*> images( LoadImages( pngnames ) );
    SortImages( images );

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
    printf( "-S, --sortby       sort type, possible values: area, width, height (default: area)\n" );
    printf( "-O, --order        sort order, possible values: asc, desc (default: desc)\n" );
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
        else if( CSTR( "-O" ) || CSTR( "--order" ) )
        {
            i++;
            if( !( CSTR( "asc" ) || CSTR( "desc" ) ) )
            {
                Error();
            }
            order = argv[i];
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
