#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include "VFS.hpp"

static std::string out;

void Error()
{
    fprintf( stderr, "Usage: mipsplit image.[etc|etc+|pvr|pvr+] [params]\n\n" );
    fprintf( stderr, "Params:\n" );
    fprintf( stderr, " -o   output directory\n" );
    exit( 1 );
}

int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        Error();
    }

    out = argv[1];
    out += ".mip";

#define CSTR(x) strcmp( argv[i], x ) == 0
    for( int i=2; i<argc; i++ )
    {
        if( CSTR( "-o" ) )
        {
            i++;
            out = argv[i];
        }
        else
        {
            Error();
        }
    }
#undef CSTR

    if( !CreateDirStruct( out ) )
    {
        fprintf( stderr, "Unable to create directory %s\n", out.c_str() );
        exit( 2 );
    }

    return 0;
}
