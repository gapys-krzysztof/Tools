#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#include "libpng/png.h"

#include "Bitmap.hpp"
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
    fseek( f, -4, SEEK_CUR );

    int32_t w, h, mips;
    int32_t fmt;
    int32_t bpp;
    int32_t mw, mh;

    if( version == 52 )
    {
        PvrHeaderV2 hdr;
        read = fread( &hdr, 1, sizeof( PvrHeaderV2 ), f );
        if( read != sizeof( PvrHeaderV2 ) ) ErrSplit();
        int format = hdr.Flags & 0xFF;
        if( hdr.HeaderSize != 52 || strncmp( hdr.FourCC, "PVR!", 4 ) != 0 || !( format == 0x0C || format == 0x0D || format == 0x18 || format == 0x19 || format == 0x36 ) )
            ErrSplit();
        if( hdr.NumSurfs != 1 ) ErrSplit();
        w = hdr.Width;
        h = hdr.Height;
        mips = hdr.MipMapCount;
        bpp = hdr.BitCount;
        assert( bpp == 2 || bpp == 4 );
        switch( format )
        {
        case 0x0C:
            fmt = 0x8C03;   // GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
            break;
        case 0x0D:
            fmt = 0x8C02;   // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
            break;
        case 0x18:
            fmt = 0x8C01;   // GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
            break;
        case 0x19:
            fmt = 0x8C00;   // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
            break;
        case 0x36:
            fmt = 0x9274;   // GL_COMPRESSED_RGB8_ETC2; 0x8D64 for GL_ETC1_RGB8_OES
            break;
        default:
            assert( false );
        }
    }
    else if( version == H_PVR3 )
    {
        PvrHeaderV3 hdr;
        read = fread( &hdr, 1, sizeof( PvrHeaderV3 ), f );
        if( read != sizeof( PvrHeaderV3 ) ) ErrSplit();
        if( !( hdr.PixelFormat[1] == 0 && hdr.PixelFormat[0] == 6 ) && !( hdr.PixelFormat[1] == 0 && hdr.PixelFormat[0] <= 3 ) )
            ErrSplit();
        if( hdr.Depth != 1 || hdr.NumSurfs != 1 || hdr.NumFaces != 1 ) ErrSplit();
        w = hdr.Width;
        h = hdr.Height;
        mips = hdr.MipMapCount - 1;
        fseek( f, hdr.MetaDataSize, SEEK_CUR );
        switch( hdr.PixelFormat[0] )
        {
        case 0:
            fmt = 0x8C01;
            bpp = 2;
            break;
        case 1:
            fmt = 0x8C03;
            bpp = 2;
            break;
        case 2:
            fmt = 0x8C00;
            bpp = 4;
            break;
        case 3:
            fmt = 0x8C02;
            bpp = 4;
            break;
        case 6:
            fmt = 0x9274;
            bpp = 4;
            break;
        default:
            assert( false );
        }
    }
    else
    {
        ErrSplit();
    }

    switch( fmt )
    {
    case 0x8C00:
    case 0x8C02:
        mw = mh = 8;
        break;
    case 0x8C01:
    case 0x8C03:
        mw = 16;
        mh = 8;
        break;
    case 0x9274:
        mw = mh = 4;
        break;
    default:
        assert( false );
    }

    FILE* o = fopen( ( out + "/meta" ).c_str(), "wb" );
    fwrite( &w, 1, 4, o );
    fwrite( &h, 1, 4, o );
    fwrite( &mips, 1, 4, o );
    fwrite( &fmt, 1, 4, o );
    fclose( o );

    for( int i=0; i<=mips; i++ )
    {
        FILE* o = fopen( ( out + "/" + std::to_string( i ) ).c_str(), "wb" );
        const auto size = ( w * h * bpp ) / 8;
        char* buf = new char[size];
        auto ret = fread( buf, 1, size, f );
        if( ret != size ) ErrSplit();
        fwrite( buf, 1, size, o );
        fclose( o );

        w = std::max( mw, w/2 );
        h = std::max( mh, h/2 );
    }
}

void SplitPNG( const char* fn )
{
    Bitmap bmp( fn );
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
    else
    {
        fseek( f, 0, SEEK_SET );
    }

    if( !CreateDirStruct( out ) )
    {
        fprintf( stderr, "Unable to create directory %s\n", out.c_str() );
        exit( 2 );
    }

    char sig[4];
    fread( sig, 1, 4, f );
    fseek( f, -4, SEEK_CUR );
    if( png_sig_cmp( (png_bytep)sig, 0, 4 ) == 0 )
    {
        SplitPNG( argv[1] );
    }
    else
    {
        SplitPVR( f );
    }

    fclose( f );

    return 0;
}
