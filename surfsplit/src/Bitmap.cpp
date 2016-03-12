#include <ctype.h>
#include <string.h>
#include <sstream>

#include "libpng/png.h"
#include "lz4/lz4.h"

#include "Bitmap.hpp"

void FatalExit(std::string const& message);
void FatalExitErrno(std::string const& message, int err);

static void my_png_error(png_structp png_ptr, png_const_charp libpng_error_message)
{
    std::ostringstream message;
    const char* fn = (const char*)png_get_error_ptr(png_ptr);
    message << "png error while processing file '" << fn << "': " << libpng_error_message;
    FatalExit(message.str());
}

Bitmap::Bitmap( const char* fn )
    : m_data( NULL )
{
    FILE* f = fopen( fn, "rb" );

    if( !f )
    {
        std::ostringstream message;
        message << "failed to open file " << fn;
        FatalExitErrno(message.str(), errno);
    }
    unsigned int sig_read = 0;
    int bit_depth, color_type, interlace_type;

    png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

    if (!png_ptr)
    {
        FatalExit("png_create_read_struct: Failed to allocate png struct");
    }

    png_set_error_fn(png_ptr, (png_voidp) fn, &my_png_error, NULL);
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
            png_set_filler( png_ptr, 0xff, PNG_FILLER_AFTER );
        }
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        png_set_gray_to_rgb( png_ptr );
        break;
    case PNG_COLOR_TYPE_RGB:
        png_set_filler( png_ptr, 0xff, PNG_FILLER_AFTER );
        break;
    default:
        break;
    }

    //printf( "Bitmap %s %ix%i\n", fn, w, h );

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
