#include <algorithm>
#include <cstdlib>

#include "Node.hpp"

Node* Node::Insert( const Rect& area, int align )
{
    Node* ret = nullptr;

    if( child[0] )
    {
        ret = child[0]->Insert( area, align );
        if( !ret )
        {
            ret = child[1]->Insert( area, align );
        }
        return ret;
    }

    int w = area.w;
    int h = area.h;

    if( align > 1 )
    {
        w = ( w + align - 1 ) / align * align;
        h = ( h + align - 1 ) / align * align;
    }

    if( w <= rect.w && h <= rect.h )
    {
        child[0] = new Node( Rect( rect.x + w, rect.y, rect.w - w, h ) );
        child[1] = new Node( Rect( rect.x, rect.y + h, rect.w, rect.h - h ) );

        oldRect = rect;
        rect = Rect( rect.x + area.x, rect.y + area.y, area.w - area.x * 2, area.h - area.y * 2 );
        ret = this;
    }

    return ret;
}

Node* Node::InsertPadded( const BRect& img, int align, int edges )
{
    auto node = Insert( Rect( edges, edges, img.w + edges * 2, img.h + edges * 2 ), align );
    if( node )
    {
        node->imgData = img;
    }
    return node;
}

void Node::RemoveChildren()
{
    if( child[0] ) child[0]->RemoveChildren();
    if( child[1] ) child[1]->RemoveChildren();

    delete child[0];
    delete child[1];
    child[0] = nullptr;
    child[1] = nullptr;

    rect = oldRect;
}

bool Node::AssetFits( const std::vector<BRect>& assetRects, int edges, int align )
{
    bool assetFits = true;
    std::vector<Node*> children;

    for( auto& r : assetRects )
    {
        Node* p = Insert( Rect( edges, edges, r.w + edges * 2, r.h + edges * 2 ), align );

        if( !p )
        {
            assetFits = false;
            break;
        }

        children.push_back( p );
    }

    // cleanup all the children and revert the tree to previous state
    for( auto rIter = children.crbegin(); rIter != children.crend(); ++rIter )
    {
        (*rIter)->RemoveChildren();
    }

    return assetFits;
}
