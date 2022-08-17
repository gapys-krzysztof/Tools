#ifndef __DARKRL__TEXTURE_HPP__
#define __DARKRL__TEXTURE_HPP__

#if defined _WIN32
#  include <windows.h>
#endif
#if defined __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

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
