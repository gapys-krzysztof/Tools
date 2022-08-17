#include <algorithm>
#include <assert.h>
#include <functional>
#include <math.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#include "libpng/png.h"

#include "Bitmap.hpp"
#include "DdsHeader.hpp"
#include "PvrHeader.hpp"
#include "VFS.hpp"

static std::string out;

void Error()
{
    fprintf( stderr, "Usage: mipsplit image.[etc|etc+|pvr|pvr+|dds] [params]\n\n" );
    fprintf( stderr, "Params:\n" );
    fprintf( stderr, " -o   output directory\n" );
    exit( 1 );
}

void ErrSplit()
{
    fprintf( stderr, "Error reading file.\n" );
    exit( 1 );
}

struct StreamingImageHeader
{
    uint32_t w, h;
    uint32_t mips;
    uint32_t fmt;
};

struct StreamingImageCallbackData
{
    uint32_t w, h;
    int index;
    FILE* output;
};

using StreamingImageCallback = std::function<bool( StreamingImageCallbackData& data )>;

bool CreateStreamingImage( const std::string& path, const StreamingImageHeader& header, const StreamingImageCallback& callback, uint32_t mw = 1, uint32_t mh = 1 )
{
    FILE* o = fopen( ( path + "meta" ).c_str(), "wb" );
    fwrite( &header.w, 1, 4, o );
    fwrite( &header.h, 1, 4, o );
    fwrite( &header.mips, 1, 4, o );
    fwrite( &header.fmt, 1, 4, o );
    fclose( o );

    auto w = header.w;
    auto h = header.h;
    char mip_path[4096];

    for( uint32_t i = 0; i <= header.mips; i++ )
    {
        StreamingImageCallbackData data;
        sprintf( mip_path, "%s%d", out.c_str(), i );
        data.output = fopen( mip_path, "wb" );
        data.w = w;
        data.h = h;
        data.index = i;

        bool result = callback( data );

        fclose( data.output );

        if( !result )
            return false;

        w = std::max( mw, w / 2 );
        h = std::max( mh, h / 2 );
    }

    return true;
}

void SplitDDS( FILE* f )
{
    enum { hCaps        = 0x00000001 };
    enum { hPixelFormat = 0x00001000 };
    enum { hMipMapCount = 0x00020000 };
    enum { hLinearSize  = 0x00080000 };
    enum { fFourCC      = 0x00000004 };

    DdsHeader header;
    auto read = fread( &header, 1, sizeof( DdsHeader ), f );
    if( read != sizeof( DdsHeader ) )
        ErrSplit();

    if( header.Size != 124 )
        ErrSplit();
    if( ( header.PixelFormat.Flags & fFourCC ) == 0 )
        ErrSplit();

    uint32_t w, h, mips;
    uint32_t fmt;
    uint32_t bpp;

    w = header.Width;
    h = header.Height;
    mips = 0;
    if( !( ( header.Flags & hMipMapCount ) == 0 ) )
    {
        mips = header.MipMapCount - 1;
    }

    auto size = ( w / 4 ) * ( h / 4 ) * 8;

    if( strncmp( "DXT1", header.PixelFormat.FourCC, 4 ) == 0 )
    {
        fmt = 0x83F0; //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        bpp = 4;
    }
    else if( strncmp( "DXT5", header.PixelFormat.FourCC, 4 ) == 0 )
    {
        fmt = 0x83F3; //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        bpp = 8;
        size *= 2;
    }
    else if( strncmp( "DX10", header.PixelFormat.FourCC, 4 ) == 0 )
    {
        enum { dTexture2D    = 3 };
        enum { fBC1Typeless  = 70 };
        enum { fBC1UNorm     = 71 };
        enum { fBC1UNormSRGB = 72 };
        enum { fBC3Typeless  = 76 };
        enum { fBC3UNorm     = 77 };
        enum { fBC3UNormSRGB = 78 };

        DdsHeader10 header10;
        if( fread( &header10, 1, sizeof( DdsHeader10 ), f ) != sizeof( DdsHeader10 ) )
            ErrSplit();

        if( header10.Dimension != dTexture2D )
            ErrSplit();

        switch( header10.Format )
        {
        case fBC1Typeless:
        case fBC1UNorm:
        case fBC1UNormSRGB:
            fmt = 0x83F0; //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
            bpp = 4;
            break;
        case fBC3Typeless:
        case fBC3UNorm:
        case fBC3UNormSRGB:
            fmt = 0x83F3; //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
            bpp = 8;
            size *= 2;
            break;
        default:
            ErrSplit();
            break;
        }
    }
    else
    {
        ErrSplit();
    }

    auto mipmapCallback = [bpp, f]( StreamingImageCallbackData& data )
    {
        auto size = ( data.w * data.h * bpp ) / 8;
        auto buffer = std::make_unique<char[]>( size );

        auto ret = fread( buffer.get(), 1, size, f );
        if( ret != size )
            return false;

        fwrite( buffer.get(), 1, size, data.output );

        return true;
    };

    StreamingImageHeader outheader{ w, h, mips, fmt };
    if( !CreateStreamingImage( out, outheader, mipmapCallback ) )
    {
        ErrSplit();
    }
}

void SplitPVR( FILE* f )
{
    uint32_t version;
    auto read = fread( &version, 1, 4, f );
    if( read != 4 ) ErrSplit();
    fseek( f, -4, SEEK_CUR );

    uint32_t w, h, mips;
    uint32_t fmt;
    uint32_t bpp;
    uint32_t mw, mh;

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
        if( hdr.PixelFormat[1] != 0 ) ErrSplit();
        if( hdr.Depth != 1 || hdr.NumSurfs != 1 || hdr.NumFaces != 1 ) ErrSplit();
        w = hdr.Width;
        h = hdr.Height;
        mips = hdr.MipMapCount - 1;
        fseek( f, hdr.MetaDataSize, SEEK_CUR );
        switch( hdr.PixelFormat[0] )
        {
        case 0:
            fmt = 0x8C01;   // GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
            bpp = 2;
            break;
        case 1:
            fmt = 0x8C03;   // GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
            bpp = 2;
            break;
        case 2:
            fmt = 0x8C00;   // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
            bpp = 4;
            break;
        case 3:
            fmt = 0x8C02;   // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
            bpp = 4;
            break;
        case 6:             // etc1
        case 22:
            fmt = 0x9274;   // GL_COMPRESSED_RGB8_ETC2
            bpp = 4;
            break;
        case 23:
            fmt = 0x9278;   // GL_COMPRESSED_RGBA8_ETC2_EAC
            bpp = 8;
            break;
        default:
            ErrSplit();
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
    case 0x9278:
        mw = mh = 4;
        break;
    default:
        assert( false );
    }

    StreamingImageHeader header{ w, h, mips, fmt };

    auto mipmapCallback = [bpp, f]( StreamingImageCallbackData& data )
    {
        auto size = ( data.w * data.h * bpp ) / 8;
        auto buffer = std::make_unique<char[]>( size );

        auto ret = fread( buffer.get(), 1, size, f );
        if ( ret != size ) return false;

        fwrite( buffer.get(), 1, size, data.output );

        return true;
    };

    if( !CreateStreamingImage( out, header, mipmapCallback, mw, mh ) )
    {
        ErrSplit();
    }
}

void WriteImageDataPNG( FILE* f, uint32_t* data, int32_t w, int32_t h, int32_t fmt )
{
    if( fmt == 0x1908 )
    {
        fwrite( data, 1, w * h * sizeof( uint32_t ), f );
    }
    else
    {
        auto ptr = data;
        for( int i = 0; i < w * h; i++ )
        {
            fwrite( ptr++, 1, 3, f );
        }
    }
}

void SplitPNG( const char* fn )
{
    Bitmap bmp( fn );

    bool alpha = bmp.Alpha();
    uint32_t w = bmp.Size().x;
    uint32_t h = bmp.Size().y;
    uint32_t fmt = alpha ? 0x1908 : 0x1907;    // GL_RGBA, GL_RGB
    uint32_t mips = (int)floor( log2( std::max( w, h ) ) );

    auto mipmapCallback = [fmt, &bmp]( StreamingImageCallbackData& data )
    {
        if( data.index == 0 )
        {
            WriteImageDataPNG( data.output, bmp.Data(), data.w, data.h, fmt );
            return true;
        }

        Bitmap tmp( data.w, data.h );

        if( data.w < 2 || data.h < 2 )
        {
            memset( tmp.Data(), 0, sizeof( uint32 ) * data.w * data.h );
        }
        else
        {
            auto dst = tmp.Data();
            auto src1 = bmp.Data();
            auto src2 = src1 + bmp.Size().x;
            for( uint32_t j=0; j<data.h; j++ )
            {
                for( uint32_t i=0; i<data.w; i++ )
                {
                    int r = ( ( *src1 & 0x000000FF ) + ( *(src1+1) & 0x000000FF ) + ( *src2 & 0x000000FF ) + ( *(src2+1) & 0x000000FF ) ) / 4;
                    int g = ( ( ( *src1 & 0x0000FF00 ) + ( *(src1+1) & 0x0000FF00 ) + ( *src2 & 0x0000FF00 ) + ( *(src2+1) & 0x0000FF00 ) ) / 4 ) & 0x0000FF00;
                    int b = ( ( ( *src1 & 0x00FF0000 ) + ( *(src1+1) & 0x00FF0000 ) + ( *src2 & 0x00FF0000 ) + ( *(src2+1) & 0x00FF0000 ) ) / 4 ) & 0x00FF0000;
                    int a = ( ( ( ( ( *src1 & 0xFF000000 ) >> 8 ) + ( ( *(src1+1) & 0xFF000000 ) >> 8 ) + ( ( *src2 & 0xFF000000 ) >> 8 ) + ( ( *(src2+1) & 0xFF000000 ) >> 8 ) ) / 4 ) & 0x00FF0000 ) << 8;
                    *dst++ = r | g | b | a;
                    src1 += 2;
                    src2 += 2;
                }
                src1 += bmp.Size().x;
                src2 += bmp.Size().x;
            }
        }

        auto ptr = tmp.Data();
        WriteImageDataPNG( data.output, ptr, data.w, data.h, fmt );
        bmp = std::move( tmp );

        return true;
    };

    StreamingImageHeader header{ w, h, mips, fmt };

    if( !CreateStreamingImage( out, header, mipmapCallback ) )
    {
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

    if( out[out.size()-1] != '/' )
    {
        out += '/';
    }

    bool pvrplus = false;

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
        fseek( f, 1, SEEK_CUR );
        pvrplus = true;
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
    else if( strncmp( sig, "DDS ", 4 ) == 0 )
    {
        SplitDDS( f );
    }
    else
    {
        if( pvrplus )
        {
            out += 'C';
            SplitPVR( f );
            out.pop_back();
            out += 'A';
            SplitPVR( f );
        }
        else
        {
            SplitPVR( f );
        }
    }

    fclose( f );

    return 0;
}
