#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "libpng/png.h"
#include "lz4/lz4.h"

#include "Bitmap.hpp"

Bitmap::Bitmap( int x, int y )
    : m_data( new uint32[x*y] )
    , m_size( x, y )
{
    memset( m_data, 0, sizeof( uint32 ) * x * y );
}

Bitmap::Bitmap( const Bitmap& bmp )
    : m_data( new uint32[bmp.m_size.x*bmp.m_size.y] )
    , m_size( bmp.m_size )
{
    memcpy( m_data, bmp.m_data, sizeof( uint32 ) * m_size.x * m_size.y );
}

Bitmap::Bitmap( const char* fn )
    : m_data( NULL )
{
    std::string lz4( fn );
    lz4 += ".lz4";

    FILE *f = fopen( lz4.c_str(), "rb" );
    if( f )
    {
        char fourcc[4];
        fread( fourcc, 1, 4, f );
        if( memcmp( fourcc, "raw4", 4 ) != 0 )
        {
            fprintf( stderr, "FATAL: %s has wrong fourcc", lz4.c_str() );
            exit( 1 );
        }

        fseek( f, 1, SEEK_CUR );    // alpha
        fread( &m_size.x, 1, 4, f );
        fread( &m_size.y, 1, 4, f );

        m_data = new uint32[m_size.x * m_size.y];

        int32 csize;
        fread( &csize, 1, 4, f );
        auto cbuf = new char[csize];
        fread( cbuf, 1, csize, f );
        fclose( f );

        LZ4_decompress_fast( cbuf, (char*)m_data, m_size.x * m_size.y * 4 );

        delete[] cbuf;
        return;
    }

    f = fopen( fn, "rb" );

    if( !f )
    {
        fprintf( stderr, "FATAL: cannot open %s", fn );
        exit( 1 );
    }

    unsigned int sig_read = 0;
    int bit_depth, color_type, interlace_type;

    png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    png_infop info_ptr = png_create_info_struct( png_ptr );
    setjmp( png_jmpbuf( png_ptr ) );

    png_init_io( png_ptr, f );
    png_set_sig_bytes( png_ptr, sig_read );

    png_uint_32 w, h;

    png_read_info( png_ptr, info_ptr );
    png_get_IHDR( png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, &interlace_type, NULL, NULL );

    m_size = v2i( w, h );

    png_set_strip_16( png_ptr );
    if( color_type == PNG_COLOR_TYPE_PALETTE )
    {
        png_set_palette_to_rgb( png_ptr );
    }
    else if( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
    {
        png_set_expand_gray_1_2_4_to_8( png_ptr );
    }
    if( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
    {
        png_set_tRNS_to_alpha( png_ptr );
    }
    if( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
    {
        png_set_gray_to_rgb(png_ptr);
    }

    switch( color_type )
    {
    case PNG_COLOR_TYPE_PALETTE:
        if( !png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
        {
            png_set_filler( png_ptr, 0xff, PNG_FILLER_BEFORE );
        }
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        break;
    case PNG_COLOR_TYPE_RGB:
        png_set_filler( png_ptr, 0xFF, PNG_FILLER_AFTER );
        break;
    default:
        break;
    }

    m_data = new uint32[w*h];
    uint32* ptr = m_data;
    while( h-- )
    {
        png_read_rows( png_ptr, (png_bytepp)&ptr, NULL, 1 );
        ptr += w;
    }

    png_read_end( png_ptr, info_ptr );

    png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
    fclose( f );
}

Bitmap::~Bitmap()
{
    delete[] m_data;
}

bool Bitmap::Write( const char* fn, bool alpha )
{
    FILE* f = fopen( fn, "wb" );
    if( !f ) return false;

    png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    png_infop info_ptr = png_create_info_struct( png_ptr );
    setjmp( png_jmpbuf( png_ptr ) );
    png_init_io( png_ptr, f );

    png_set_compression_level( png_ptr, 1 );
    png_set_filter( png_ptr, 0, PNG_FILTER_NONE );

    png_set_IHDR( png_ptr, info_ptr, m_size.x, m_size.y, 8, alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

    png_write_info( png_ptr, info_ptr );

    if( !alpha )
    {
        png_set_filler( png_ptr, 0, PNG_FILLER_AFTER );
    }

    uint32* ptr = m_data;
    for( int i=0; i<m_size.y; i++ )
    {
        png_write_rows( png_ptr, (png_bytepp)(&ptr), 1 );
        ptr += m_size.x;
    }

    png_write_end( png_ptr, info_ptr );
    png_destroy_write_struct( &png_ptr, &info_ptr );

    fclose( f );
    return true;
}

bool Bitmap::WriteRaw( const char* fn, bool alpha )
{
    FILE* f = fopen( fn, "wb" );
    if( !f ) return false;

    const char fourcc[] = { 'r', 'a', 'w', '4' };
    fwrite( fourcc, 1, sizeof( fourcc ), f );
    uint8 a = alpha ? 1 : 0;
    fwrite( &a, 1, 1, f );
    uint32 d = m_size.x;
    fwrite( &d, 1, 4, f );
    d = m_size.y;
    fwrite( &d, 1, 4, f );

    const auto cbufsize = LZ4_compressBound( m_size.x * m_size.y * 4 );
    auto cbuf = new char[cbufsize];
    const int32 csize = LZ4_compress_default( (const char*)m_data, cbuf, m_size.x * m_size.y * 4, cbufsize );

    fwrite( &csize, 1, 4, f );
    fwrite( cbuf, 1, csize, f );

    delete[] cbuf;

    fclose( f );
    return true;
}

Bitmap& Bitmap::operator=( const Bitmap& bmp )
{
    if( m_size != bmp.m_size )
    {
        m_size = bmp.m_size;
        delete[] m_data;
        m_data = new uint32[m_size.x*m_size.y];
    }

    memcpy( m_data, bmp.m_data, sizeof( uint32 ) * m_size.x * m_size.y );

    return *this;
}

void Blit( Bitmap* _dst, const BRect& _src, const Rect& rect )
{
    uint32* src = _src.b->Data() + _src.x + _src.y * _src.b->Size().x;
    uint32* dst = _dst->Data() + rect.x + rect.y * _dst->Size().x;

    int sf = _src.b->Size().x;
    int sb = _src.b->Size().x * rect.w - 1;
    int line = _dst->Size().x - rect.w;

    if( _src.flip )
    {
        for( int y=0; y<rect.h; y++ )
        {
            for( int x=0; x<rect.w; x++ )
            {
                *dst++ = *src;
                src += sf;
            }
            dst += line;
            src -= sb;
        }
    }
    else
    {
        int line = _dst->Size().x;
        int skip = _src.b->Size().x;

        for( int y=0; y<rect.h; y++ )
        {
            memcpy( dst, src, sizeof( uint32 ) * rect.w );
            dst += line;
            src += skip;
        }
    }
}
