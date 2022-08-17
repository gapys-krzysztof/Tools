#ifndef __ATLAS_HPP__
#define __ATLAS_HPP__

#include <memory>
#include <type_traits>
#include <vector>

#include "Rect.hpp"

class Bitmap;
class Node;

class Atlas
{
public:
    enum Flags
    {
        Square     = 0x01,
        POTWidth   = 0x02,
        POTHeight  = 0x04,
        NoAlpha    = 0x08,
        SplashFill = 0x10,
        WriteRaw   = 0x20,
    };

    static const Flags DefaultFlags = Flags::SplashFill;

    Atlas( int width, int height, const std::string& prepend, int edges, int align );

    bool AddImage( const BRect& img );
    void AdjustSize( Flags flags );
    bool AssetFits( const std::vector<BRect>& assetRects );
    int BitmapSize() const;
    bool Optimize();
    bool Output( const std::string& bitmapName, const std::string& xmlName, const std::string& pathStripPrefix, int pathStripDepth, Flags flags ) const;
    void BlitImages();
    void ReportStats() const;
    bool Shrink();

private:
    void BlitTree( const Node* node );
    void SortImages();
    std::unique_ptr<Node> BuildTree( int width, int height, const std::vector<BRect>& images );

    int m_width, m_height;
    int m_edges, m_align;
    std::unique_ptr<Bitmap> m_bitmap;
    std::unique_ptr<Node> m_tree;
    std::string m_prepend;
    std::vector<BRect> m_images;
    bool m_optimized;
};

namespace
{
    using E = Atlas::Flags;
    using T = std::underlying_type_t<E>;

    inline E operator ~ ( E value )
    {
        value = static_cast<E>( ~static_cast<T>( value ) );
        return value;
    }

    inline E& operator |= ( E& lhs, E rhs )
    {
        lhs = static_cast<E>( static_cast<T>( lhs ) | static_cast<T>( rhs ) );
        return lhs;
    }

    inline E& operator &= ( E& lhs, E rhs )
    {
        lhs = static_cast<E>( static_cast<T>( lhs ) & static_cast<T>( rhs ) );
        return lhs;
    }
}

#endif
