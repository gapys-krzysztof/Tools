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
int edges = 0;
int path = -1;
bool potw = false;
bool poth = false;
bool align = false;
std::string prepend;
bool square = false;
bool noalpha = false;


std::vector<BRect> LoadImages( const std::list<std::string> pngs, const std::list<std::string> names, const std::list<std::string> rectnames )
{
    std::vector<BRect> ret;

    std::list<std::string>::const_iterator nit = names.begin();
    std::list<std::string>::const_iterator rit = rectnames.begin();

    ret.reserve( pngs.size() );
    for( std::list<std::string>::const_iterator it = pngs.begin(); it != pngs.end(); ++it, ++nit, ++rit )
    {
        Bitmap* b = new Bitmap( it->c_str() );
        FILE* f = fopen( rit->c_str(), "rb" );
        if( !f )
        {
            ret.push_back( BRect( 0, 0, b->Size().x, b->Size().y, b, *nit ) );
        }
        else
        {
            int size;
            fread( &size, 1, 4, f );
            for( int i=0; i<size; i++ )
            {
                BRect r( 0, 0, 0, 0, b, *nit );
                fread( &r, 1, sizeof( uint16 ) * 4, f );
                ret.push_back( r );
            }
            fclose( f );
        }
    }

    return ret;
}

void SortImages( std::vector<BRect>& images )
{
    struct
    {
        bool operator()( const BRect& i1, const BRect& i2 )
        {
            if( i1.h != i2.h )
            {
                return i1.h > i2.h;
            }
            else
            {
                return i1.w > i2.w;
            }
        }
    } Comparator;

    std::sort( images.begin(), images.end(), Comparator );
}

void FindFlipped( std::vector<BRect>& images )
{
    for( std::vector<BRect>::iterator it = images.begin(); it != images.end(); ++it )
    {
        if( it->h > it->w )
        {
            it->flip = true;
            std::swap( it->h, it->w );
        }
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

    std::vector<BRect> images( LoadImages( pngnames, names, rectnames ) );
    FindFlipped( images );
    SortImages( images );

    Bitmap* b = new Bitmap( size, size );
    Node* tree = new Node( Rect( 0, 0, size, size ) );

    int maxWidth = 1;
    int maxHeight = 1;

    for( std::vector<BRect>::const_iterator it = images.begin(); it != images.end(); ++it )
    {
        if( edges != 0 ) {}
        if( align ) {}

        Node* uv = tree->Insert( Rect( 0, 0, it->w, it->h ), align );
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
