#include "Video.hpp"

#ifdef BUILD_VIS

#include <SDL.h>

#include "Texture.hpp"

void ShowBitmap( Bitmap* bmp, const std::vector<Rect>& rects, const std::vector<DupRect>& duprects )
{
    SDL_Init( SDL_INIT_VIDEO );
    auto win = SDL_CreateWindow( "surfsplit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, bmp->Size().x, bmp->Size().y, SDL_WINDOW_OPENGL );
    SDL_GL_CreateContext( win );

    Texture tex( bmp );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, tex );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glClearColor( 0.5, 0.5, 0.5, 1 );

    float xr = 2.f / bmp->Size().x;
    float yr = 2.f / bmp->Size().y;

    SDL_Event e;
    for(;;)
    {
        while( SDL_PollEvent( &e ) )
        {
            switch( e.type )
            {
            case SDL_QUIT:
            case SDL_KEYDOWN:
                return;
            default:
                break;
            }
        }

        glClear( GL_COLOR_BUFFER_BIT );

        glBegin( GL_QUADS );
        glTexCoord2f( 0, 1 );
        glVertex2f( -1, -1 );
        glTexCoord2f( 1, 1 );
        glVertex2f( 1, -1 );
        glTexCoord2f( 1, 0 );
        glVertex2f( 1, 1 );
        glTexCoord2f( 0, 0 );
        glVertex2f( -1, 1 );
        glEnd();

        glDisable( GL_TEXTURE_2D );
        glColor4f( 0, 1, 0, 0.5f );
        for( auto it = rects.begin(); it != rects.end(); ++it )
        {
            glBegin( GL_LINE_STRIP );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glVertex2f( ( it->x + it->w + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glVertex2f( ( it->x + it->w + 0.5 ) * xr - 1, 1 - ( ( it->y + it->h + 0.5 ) * yr ) );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + it->h + 0.5 ) * yr ) );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glEnd();
        }
        glColor4f( 1, 0, 0, 0.25f );
        for( auto it = duprects.begin(); it != duprects.end(); ++it )
        {
            for( int i=0; i<it->xy.size(); i++ )
            {
                int x = it->xy[i].x;
                int y = it->xy[i].y;
                int w = it->xy[i].w;
                int h = it->xy[i].h;
                glBegin( GL_LINE_STRIP );
                glVertex2f( ( x + 0.5 ) * xr - 1, 1 - ( ( y + 0.5 ) * yr ) );
                glVertex2f( ( x + w + 0.5 ) * xr - 1, 1 - ( ( y + 0.5 ) * yr ) );
                glVertex2f( ( x + w + 0.5 ) * xr - 1, 1 - ( ( y + h + 0.5 ) * yr ) );
                glVertex2f( ( x + 0.5 ) * xr - 1, 1 - ( ( y + h + 0.5 ) * yr ) );
                glVertex2f( ( x + 0.5 ) * xr - 1, 1 - ( ( y + 0.5 ) * yr ) );
                glEnd();
            }
        }
        glColor4f( 1, 1, 1, 0.5f );
        for( auto it = duprects.begin(); it != duprects.end(); ++it )
        {
            glBegin( GL_LINE_STRIP );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glVertex2f( ( it->x + it->w + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glVertex2f( ( it->x + it->w + 0.5 ) * xr - 1, 1 - ( ( it->y + it->h + 0.5 ) * yr ) );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + it->h + 0.5 ) * yr ) );
            glVertex2f( ( it->x + 0.5 ) * xr - 1, 1 - ( ( it->y + 0.5 ) * yr ) );
            glEnd();
        }
        glColor4f( 1, 1, 1, 1 );
        glEnable(GL_TEXTURE_2D );

        SDL_GL_SwapWindow( win );
    }
}

#else

void ShowBitmap( Bitmap* bmp, const std::vector<Rect>& rects, const std::vector<DupRect>& duprects )
{
}

#endif
