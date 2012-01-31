#ifndef __DARKRL__TEXTURE_HPP__
#define __DARKRL__TEXTURE_HPP__

#ifdef _WIN32
#  include <windows.h>
#endif
#include <GL/gl.h>

#include "Bitmap.hpp"
#include "Vector.hpp"

class Texture
{
public:
    Texture( Bitmap* bitmap );
    ~Texture();

    operator GLuint() const { return m_texture; }

    const v2i& Size() const { return m_size; }

private:
    GLuint m_texture;
    v2i m_size;
};

#endif
