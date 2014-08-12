#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#include "PvrHeader.hpp"
#include "VFS.hpp"

static std::string out;

void Error()
{
    fprintf( stderr, "Usage: mipsplit image.[etc|etc+|pvr|pvr+] [params]\n\n" );
    fprintf( stderr, "Params:\n" );
    fprintf( stderr, " -o   output directory\n" );
    exit( 1 );
}

void ErrSplit()
{
    fprintf( stderr, "Error reading file.\n" );
    exit( 1 );
}

void SplitPVR( FILE* f )
{
    uint32_t version;
    auto read = fread( &version, 1, 4, f );
    if( read != 4 ) ErrSplit();

    if( version == 52 )
    {
        PvrHeaderV2 hdr;
        read = fread( &hdr, 1, sizeof( PvrHeaderV2 ), f );
        if( read != sizeof( PvrHeaderV2 ) ) ErrSplit();
        int format = hdr.Flags & 0xFF;
        if( hdr.HeaderSize != 52 || strncmp( hdr.FourCC, "PVR!", 4 ) != 0 || !( format == 0x0C || format == 0x0D || format == 0x18 || format == 0x19 || format == 0x36 ) )
            ErrSplit();
    }
    else if( version == H_PVR3 )
    {
        PvrHeaderV3 hdr;
        read = fread( &hdr, 1, sizeof( PvrHeaderV3 ), f );
        if( read != sizeof( PvrHeaderV3 ) ) ErrSplit();
        if( !( hdr.PixelFormat[1] == 0 && hdr.PixelFormat[0] == 6 ) || !( hdr.PixelFormat[1] == 0 && hdr.PixelFormat[0] <= 3 ) )
            ErrSplit();
    }
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

    FILE* f = fopen( argv[1], "rb" );
    char fourcc[4];
    auto ret = fread( fourcc, 1, 4, f );
    if( ret != 4 )
    {
        fprintf( stderr, "Invalid file.\n" );
        exit( 3 );
    }
    if( strncmp( fourcc, "PVR+", 4 ) == 0 )
    {
        fprintf( stderr, "PVR+ is not currently supported.\n" );
        exit( -1 );
    }

    if( !CreateDirStruct( out ) )
    {
        fprintf( stderr, "Unable to create directory %s\n", out.c_str() );
        exit( 2 );
    }

    SplitPVR( f );

    fclose( f );

    return 0;
}
