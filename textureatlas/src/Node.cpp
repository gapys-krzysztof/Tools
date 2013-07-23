#include <cstdlib>

#include "Node.hpp"

Node* Node::Insert( const Rect& area, bool align )
{
    Node* ret = NULL;

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

    if( align )
    {
        w = ( w + 3 ) & ~0x3;
        h = ( h + 3 ) & ~0x3;
    }

    if( w <= rect.w && h <= rect.h )
    {
        child[0] = new Node( Rect( rect.x + w, rect.y, rect.w - w, h ) );
        child[1] = new Node( Rect( rect.x, rect.y + h, rect.w, rect.h - h ) );

        rect = Rect( rect.x + area.x, rect.y + area.y, area.w - area.x * 2, area.h - area.y * 2 );
        ret = this;
    }

    return ret;
}
