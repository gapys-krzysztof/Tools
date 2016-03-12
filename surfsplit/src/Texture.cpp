#ifdef BUILD_VIS

#include "Texture.hpp"

Texture::Texture( Bitmap* bitmap )
    : m_size( bitmap->Size() )
{
    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bitmap->Size().x, bitmap->Size().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->Data() );
}

Texture::~Texture()
{
    glDeleteTextures( 1, &m_texture );
}

#endif
