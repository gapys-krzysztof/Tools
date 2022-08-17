#include <algorithm>
#include <map>
#include <SDL.h>

#include "Texture.hpp"
#include "Video.hpp"

struct SurfRect
{
    uint16_t x, y, w, h;
    typedef std::vector<uint16_t> Points;
    Points p1;
    Points p2;
};

bool rectsHor;

static std::vector<SurfRect> CalculatePoints( const std::vector<Rect>& rects )
{
    std::vector<SurfRect> ret;
    ret.reserve( rects.size() );

    for( auto& r : rects )
    {
        ret.emplace_back( SurfRect { r.x, r.y, r.w, r.h } );
    }

    bool alignmentKnown = false;

    for( int i = 0; i < ret.size(); ++i )
    {
        SurfRect& ri = ret[i];
        uint16_t rix2 = ri.x+ri.w;
        uint16_t riy2 = ri.y+ri.h;
        for ( int j = i+1; j < ret.size(); ++j )
        {
            SurfRect& rj = ret[j];

            // vertical
            bool added = false;
            if( ri.x == rj.x + rj.w )
            { // p1
                if( ri.y < rj.y && riy2 > rj.y )
                {
                    ri.p1.push_back( rj.y );
                    added = true;
                }
                if( rj.y < riy2 && rj.y + rj.h > riy2 )
                {
                    rj.p2.push_back( riy2 );
                    added = true;
                }
                if( ri.y < rj.y + rj.h && riy2 > rj.y + rj.h )
                {
                    ri.p1.push_back( rj.y + rj.h );
                    added = true;
                }
                if( rj.y < ri.y && rj.y + rj.h > ri.y )
                {
                    rj.p2.push_back( ri.y );
                    added = true;
                }
            }

            if( rix2 == rj.x )
            { //p2
                if( ri.y < rj.y && riy2 > rj.y )
                {
                    ri.p2.push_back( rj.y );
                    added = true;
                }
                if( rj.y < riy2 && rj.y + rj.h > riy2 )
                {
                    rj.p1.push_back( riy2 );
                    added = true;
                }
                if( ri.y < rj.y + rj.h && riy2 > rj.y + rj.h )
                {
                    ri.p2.push_back( rj.y + rj.h );
                    added = true;
                }
                if( rj.y < ri.y && rj.y + rj.h > ri.y )
                {
                    rj.p1.push_back( ri.y );
                    added = true;
                }
            }
            if( added )
            {
                alignmentKnown = true;
                rectsHor = false;
            }
            added = false;

            // horizontal
            if( ri.y == rj.y + rj.h )
            { // p1
                if( ri.x < rj.x && rix2 > rj.x )
                {
                    ri.p1.push_back( rj.x );
                    added = true;
                }
                if( rj.x < rix2 && rj.x + rj.w > rix2 )
                {
                    rj.p2.push_back( rix2 );
                    added = true;
                }
                if ( ri.x < rj.x + rj.w && rix2 > rj.x + rj.w )
                {
                    ri.p1.push_back( rj.x + rj.w );
                    added = true;
                }
                if( rj.x < ri.x && rj.x + rj.w > ri.x )
                {
                    rj.p2.push_back( ri.x );
                    added = true;
                }
            }

            if( riy2 == rj.y )
            { //p2
                if( ri.x < rj.x && rix2 > rj.x )
                {
                    ri.p2.push_back( rj.x );
                    added = true;
                }
                if( rj.x < rix2 && rj.x + rj.w > rix2 )
                {
                    rj.p1.push_back( rix2 );
                    added = true;
                }
                if( ri.x < rj.x + rj.w && rix2 > rj.x + rj.w )
                {
                    ri.p2.push_back( rj.x + rj.w );
                    added = true;
                }
                if( rj.x < ri.x && rj.x + rj.w > ri.x )
                {
                    rj.p1.push_back( ri.x );
                    added = true;
                }
            }
            if( added )
            {
                alignmentKnown = true;
                rectsHor = true;
            }
        }
    }

    for( int i = 0; i < ret.size(); ++i )
    {
        SurfRect& ri = ret[i];
        uint16_t rix2 = ri.x+ri.w;
        uint16_t riy2 = ri.y+ri.h;
        if( rectsHor )
        {
            ri.p1.push_back( ri.x );
            ri.p2.push_back( ri.x );
            ri.p1.push_back( rix2 );
            ri.p2.push_back( rix2 );
        }
        else
        {
            ri.p1.push_back( ri.y );
            ri.p2.push_back( ri.y );
            ri.p1.push_back( riy2 );
            ri.p2.push_back( riy2 );
        }

        std::sort( ri.p1.begin(), ri.p1.end() );
        ri.p1.resize( std::unique( ri.p1.begin(), ri.p1.end() ) - ri.p1.begin() );
        std::sort( ri.p2.begin(), ri.p2.end() );
        ri.p2.resize( std::unique( ri.p2.begin(), ri.p2.end() ) - ri.p2.begin() );
    }

    return ret;
}

struct SurfUV
{
    unsigned int xyref;
};

struct SurfTri
{
    union
    {
        struct
        {
            unsigned int u1, u2, u3;
        };
        unsigned int u[3];
    };
};

struct Vectorf
{
    float x, y;
};

struct Vector16
{
    union
    {
        struct
        {
            uint16_t x, y;
        };
        uint16_t v[2];
    };
};

bool operator<( const Vector16& l, const Vector16& r ) { return l.x == r.x ? ( l.y < r.y ) : ( l.x < r.x ); }

struct SurfGeo
{
    std::vector<Vectorf> xy;
    std::vector<SurfUV> uv;
    std::vector<SurfTri> t;
};

static SurfGeo CalcGeo( const std::vector<SurfRect>& rects )
{
    using Point = Vector16;

    SurfGeo geo;
    std::map<Point, unsigned int> xymap;

    for( int i = 0; i < rects.size(); ++i )
    {
        const SurfRect& r = rects[i];
        std::vector<Point> points;
        points.reserve( r.p1.size() + r.p2.size() );
        if( rectsHor )
        {
            for( auto& p : r.p1 ) points.emplace_back( Vector16 { p, r.y } );
            for( auto& p : r.p2 ) points.emplace_back( Vector16 { p, uint16_t( r.y+r.h ) } );
            std::sort( points.begin(), points.end(), []( Point& p1, Point& p2 ) { return p1.x < p2.x; } );
        }
        else
        {
            for( auto& p : r.p1 ) points.emplace_back( Vector16 { r.x, p } );
            for( auto& p : r.p2 ) points.emplace_back( Vector16 { uint16_t( r.x+r.w ), p } );
            std::sort( points.begin(), points.end(), []( Point& p1, Point& p2 ) { return p1.y < p2.y; } );
        }

        std::map<Point, unsigned int> uvmap;
        for( auto& p : points )
        {
            unsigned int xy;
            auto it = xymap.find( p );
            if( it == xymap.end() )
            {
                xy = geo.xy.size();
                xymap.emplace( p, xy );
                geo.xy.emplace_back( Vectorf { float( p.x ), float( p.y ) } );
            }
            else
            {
                xy = it->second;
            }

            uvmap.emplace( p, geo.uv.size() );
            geo.uv.emplace_back( SurfUV{ xy } );
        }

        auto size = points.size();

        int pidx = rectsHor ? 1 : 0;
        if( points[size-1].v[pidx] != points[size-3].v[pidx] )
        {
            std::swap( points[size-1], points[size-2] );
        }

        int i1 = 0;
        int i2 = 1;
        for( int i=2; i<size; i++ )
        {
            unsigned int v1 = uvmap[points[i1]];
            unsigned int v2 = uvmap[points[i2]];
            unsigned int v3 = uvmap[points[i]];

            geo.t.emplace_back( SurfTri{ v1, v2, v3 } );

            if( points[i1].v[pidx] == points[i].v[pidx] )
            {
                i1 = i;
            }
            else
            {
                i2 = i;
            }
        }
    }

    return geo;
}

void ShowBitmap( Bitmap* bmp, const std::vector<Rect>& rects, const std::vector<DupRect>& duprects )
{
    auto sr = CalculatePoints( rects );
    auto geo = CalcGeo( sr );

    SDL_Init( SDL_INIT_VIDEO );
    auto win = SDL_CreateWindow( "surfsplit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, bmp->Size().x, bmp->Size().y, SDL_WINDOW_OPENGL );
    SDL_GL_CreateContext( win );

    Texture tex( bmp );

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LINE_SMOOTH );
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

        glColor4f( 1, 0, 0, 0.5f );
        for( auto& t : geo.t )
        {
            glBegin( GL_LINE_STRIP );
            for( int i=0; i<4; i++ )
            {
                glVertex2f( ( geo.xy[geo.uv[t.u[i%3]].xyref].x + 0.5 ) * xr - 1, 1 - ( ( geo.xy[geo.uv[t.u[i%3]].xyref].y + 0.5 ) * yr ) );
            }
            glEnd();
        }
        glColor4f( 0, 1, 0, 0.5f );
        for( auto& r : sr )
        {
            glBegin( GL_LINE_STRIP );
            glVertex2f( ( r.x + 0.5 ) * xr - 1, 1 - ( ( r.y + 0.5 ) * yr ) );
            glVertex2f( ( r.x + r.w + 0.5 ) * xr - 1, 1 - ( ( r.y + 0.5 ) * yr ) );
            glVertex2f( ( r.x + r.w + 0.5 ) * xr - 1, 1 - ( ( r.y + r.h + 0.5 ) * yr ) );
            glVertex2f( ( r.x + 0.5 ) * xr - 1, 1 - ( ( r.y + r.h + 0.5 ) * yr ) );
            glVertex2f( ( r.x + 0.5 ) * xr - 1, 1 - ( ( r.y + 0.5 ) * yr ) );
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
