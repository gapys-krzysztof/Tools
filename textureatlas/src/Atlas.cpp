#include <algorithm>
#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <unordered_set>

#include "Atlas.hpp"
#include "Bitmap.hpp"
#include "Node.hpp"
#include "Processing.hpp"

namespace
{

template<typename T>
inline T AlignPOT( T a )
{
    if( a == 0 )
    {
        return 1;
    }

    a--;
    for( int i=1; i<sizeof(T)*8; i<<=1 )
    {
        a |= a >> i;
    }
    return a + 1;
}

std::map<std::string, std::pair<int, int>> imageSizes;

}

extern std::chrono::high_resolution_clock::time_point benchData[10];

Atlas::Atlas( int width, int height, const std::string& prepend, int edges, int align )
    : m_width( width )
    , m_height( height )
    , m_edges( edges )
    , m_align( align )
    , m_prepend( prepend )
    , m_bitmap( nullptr )
    , m_tree( std::make_unique<Node>( Rect( 0, 0, m_width, m_height ) ) )
    , m_optimized( true )
{
}

bool Atlas::AddImage( const BRect& img )
{
    Node* uv = m_tree->InsertPadded( img, m_align, m_edges );
    if( uv == nullptr ) return false;
    m_optimized = false;
    m_images.push_back( img );
    return true;
}

void Atlas::AdjustSize( Flags flags )
{
    if( flags & POTWidth )
    {
        m_width = AlignPOT( m_width );
    }

    if( flags & POTHeight )
    {
        m_height = AlignPOT( m_height );
    }

    if( flags & Square )
    {
        int sq = std::max( m_width, m_height );
        m_width = sq;
        m_height = sq;
    }
}

bool Atlas::AssetFits( const std::vector<BRect>& assetRects )
{
    return m_tree->AssetFits( assetRects, m_edges, m_align );
}

int Atlas::BitmapSize() const
{
    if( !m_bitmap ) return 0;
    return m_bitmap->Size().x * m_bitmap->Size().y;
}

bool Atlas::Optimize()
{
    if( m_optimized ) return false;
    m_optimized = true;

    SortImages();
    auto tree = BuildTree( m_width, m_height, m_images );
    if( !tree ) return false;

    m_tree = std::move( tree );
    return true;
}

bool Atlas::Output( const std::string& bitmapName, const std::string& xmlName, const std::string& pathStripPrefix, int pathStripDepth, Flags flags ) const
{
    struct Data
    {
        Data( const BRect* _br, const Rect* _pr ) : br( _br ), pr( _pr ) {}
        const BRect* br;
        const Rect* pr;
    };

    std::map<std::string, std::vector<Data> > irmap;

    std::function<void ( const Node* )> traverse = [ &irmap, &traverse ]( const Node* node )
    {
        if( !node->child[0] ) return;

        irmap[node->imgData.name].push_back( Data( &node->imgData, &node->rect ) );
        traverse( node->child[0] );
        traverse( node->child[1] );
    };

    traverse( m_tree.get() );

    if( irmap.empty() )
    {
        printf( "Skipping output of an empty atlas: %s\n", bitmapName.c_str() );
        return true;
    }

    FILE* f = fopen( xmlName.c_str(), "wb" );
    fprintf( f, "<?xml version=\"1.0\"?>\n" );
    fprintf( f, "<atlas height=\"%i\" width=\"%i\">\n", m_bitmap->Size().x, m_bitmap->Size().y );
    for( const auto& data : irmap )
    {
        std::string id;
        if( ( pathStripDepth == -1 ) && pathStripPrefix.empty() )
        {
            id = m_prepend + data.first;
        }
        else if( !pathStripPrefix.empty() )
        {
            // If prefix to be stripped is found at the very beginning of the file path then strip
            // it. If it occurs anywhere else, leave the path unchanged.

            if( !data.first.find( pathStripPrefix ) )
            {
                id = m_prepend + data.first.substr( pathStripPrefix.length() );
            }
            else
            {
                id = m_prepend + data.first;
            }
        }
        else
        {
            size_t pos = 0;
            for( int i = 0; i < pathStripDepth; i++ )
            {
                pos = data.first.find( '/', pos ) + 1;
            }
            id = m_prepend + data.first.substr( pos );
        }

        auto it = imageSizes.find( id );
        if( it == imageSizes.end() )
        {
            imageSizes.emplace( id, std::make_pair( data.second.begin()->br->b->Size().x, data.second.begin()->br->b->Size().y ) );
        }
        else
        {
            if( it->second.first != data.second.begin()->br->b->Size().x ||
                it->second.second != data.second.begin()->br->b->Size().y )
            {
                fclose( f );
                printf( "\n" );
                fprintf( stderr, "Image size difference detected when processing atlas %s\n", bitmapName.c_str() );
                fprintf( stderr, "Asset name: %s\n", id.c_str() );
                fprintf( stderr, "Expected: %i x %i\n", it->second.first, it->second.second );
                fprintf( stderr, "Actual:   %i x %i\n", data.second.begin()->br->b->Size().x, data.second.begin()->br->b->Size().y );
                return false;
            }
        }

        fprintf( f, "  <asset id=\"%s\" rects=\"%zu\" w=\"%i\" h=\"%i\">\n", id.c_str(), data.second.size(), data.second.begin()->br->b->Size().x, data.second.begin()->br->b->Size().y );
        for( const auto& image : data.second )
        {
            fprintf( f, "    <rect f=\"%i\" ax=\"%i\" ay=\"%i\" x=\"%i\" y=\"%i\" w=\"%i\" h=\"%i\"/>\n",
                image.br->flip ? 1 : 0,
                image.pr->x,
                image.pr->y,
                image.br->x,
                image.br->y,
                image.br->flip ? image.br->h : image.br->w,
                image.br->flip ? image.br->w : image.br->h
            );
        }
        fprintf( f, "  </asset>\n" );
    }
    fprintf( f, "</atlas>\n" );
    fclose( f );

    benchData[6] = std::chrono::high_resolution_clock::now();
    if( flags & SplashFill )
    {
        ::SplashFill( m_bitmap.get() );
    }
    benchData[7] = std::chrono::high_resolution_clock::now();

    benchData[8] = std::chrono::high_resolution_clock::now();
    if( flags & WriteRaw )
    {
        m_bitmap->WriteRaw( ( bitmapName + ".raw" ).c_str(), !(flags & NoAlpha) );
    }
    else
    {
        m_bitmap->Write( ( bitmapName + ".png" ).c_str(), !(flags & NoAlpha) );
    }
    benchData[9] = std::chrono::high_resolution_clock::now();

    return true;
}

void Atlas::SortImages()
{
    std::stable_sort( m_images.begin(), m_images.end(), []( const BRect& i1, const BRect& i2 ) { if( i1.h != i2.h ) return i1.h > i2.h; else return i1.w > i2.w; } );
}

void Atlas::BlitTree( const Node* node )
{
    if( !node->child[0] ) return;

    const auto& img = node->imgData;
    const auto& rect = node->rect;
    Blit( m_bitmap.get(), img, rect );
    for( int i = 0; i < m_edges; i++ )
    {
        BRect br( img );
        Blit( m_bitmap.get(), br, Rect( rect.x - i - 1, rect.y, 1, rect.h ) );

        if( br.flip )
        {
            br.y = img.y + img.w - 1;
        }
        else
        {
            br.x = img.x + img.w - 1;
        }
        Blit( m_bitmap.get(), br, Rect( rect.x + rect.w + i, rect.y, 1, rect.h ) );
    }

    for( int i = 0; i < m_edges; i++ )
    {
        BRect br( img );
        Blit( m_bitmap.get(), br, Rect( rect.x, rect.y - i - 1, rect.w, 1 ) );

        if( br.flip )
        {
            br.x = img.x + img.h - 1;
        }
        else
        {
            br.y = img.y + img.h - 1;
        }
        Blit( m_bitmap.get(), br, Rect( rect.x, rect.y + rect.h + i, rect.w, 1 ) );
    }

    for( int i = 0; i < m_edges; i++ )
    {
        {
            BRect br( rect.x, rect.y - m_edges, 1, m_edges, m_bitmap.get(), "" );
            Blit( m_bitmap.get(), br, Rect( rect.x - i - 1, rect.y - m_edges, 1, m_edges ) );
        }
        {
            BRect br( rect.x, rect.y + rect.h, 1, m_edges, m_bitmap.get(), "" );
            Blit( m_bitmap.get(), br, Rect( rect.x - i - 1, rect.y + rect.h, 1, m_edges ) );
        }
        {
            BRect br( rect.x + rect.w - 1, rect.y - m_edges, 1, m_edges, m_bitmap.get(), "" );
            Blit( m_bitmap.get(), br, Rect( rect.x + rect.w + i, rect.y - m_edges, 1, m_edges ) );
        }
        {
            BRect br( rect.x + rect.w - 1, rect.y + rect.h, 1, m_edges, m_bitmap.get(), "" );
            Blit( m_bitmap.get(), br, Rect( rect.x + rect.w + i, rect.y + rect.h, 1, m_edges ) );
        }
    }

    BlitTree( node->child[0] );
    BlitTree( node->child[1] );
}

void Atlas::BlitImages()
{
    m_bitmap = std::make_unique<Bitmap>( m_width, m_height );
    BlitTree( m_tree.get() );
}

void Atlas::ReportStats() const
{
    unsigned pngStat = 0, csrStat = 0, atlStat = 0;
    std::unordered_set <Bitmap*> images;

    std::function<void ( const Node* node )> walkTree = [ this, &pngStat, &csrStat, &atlStat, &images, &walkTree ]( const Node* node )
    {
        if( !node || !node->child[0] ) return;

        if( images.insert( node->imgData.b ).second )
        {
            auto bsz = node->imgData.b->Size();
            pngStat += bsz.x * bsz.y;
        }

        csrStat += node->imgData.w * node->imgData.h;
        atlStat += node->rect.w * node->rect.h + m_edges * ( m_edges * 4 + 2 * ( node->rect.h + node->rect.w ) );

        walkTree( node->child[0] );
        walkTree( node->child[1] );
    };

    walkTree( m_tree.get() );

    printf( "PNG size: %.2f Kpx\n", pngStat / 1000.f );
    printf( "CSR size: %.2f Kpx\n", csrStat / 1000.f );
    printf( "CSR reduction: %.2f%%\n", float( csrStat ) / pngStat * 100 );
    auto as = BitmapSize();
    printf( "Atlas size: %.2f Kpx\n", as / 1000.f );
    printf( "Atlas used: %.2f Kpx\n", atlStat / 1000.f );
    printf( "Atlas fill: %.2f%%\n", float( atlStat ) / as * 100 );
}

bool Atlas::Shrink()
{
    if( m_width != m_height || m_width == 0 ) return false;

    const auto newWidth = m_width / 2, newHeight = m_height / 2;

    auto tree = BuildTree( newWidth, newHeight, m_images );
    if( !tree ) return false;

    m_tree = std::move( tree );
    m_width = newWidth;
    m_height = newHeight;
    return true;
}

std::unique_ptr<Node> Atlas::BuildTree( int width, int height, const std::vector<BRect>& images )
{
    auto tree = std::make_unique<Node>( Rect( 0, 0, width, height ) );
    for( const auto& img : images )
    {
        if( !tree->InsertPadded( img, m_align, m_edges ) ) return nullptr;
    }

    return tree;
}
