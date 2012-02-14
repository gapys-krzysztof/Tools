#include <cstdlib>

#include "Node.hpp"

Node* Node::Insert( Node* area, bool align )
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

    if( area->rect.w <= rect.w && area->rect.h <= rect.h )
    {
        Rect& ar = area->rect;
        if( align )
        {
        }
        else
        {
            child[0] = new Node( Rect( rect.x + ar.w, rect.y, rect.w - ar.w, ar.h ) );
            child[1] = new Node( Rect( rect.x, rect.y + ar.h, rect.w, rect.h - ar.h ) );
        }
        ret = new Node( Rect( rect.x, rect.y, ar.w, ar.h ) );
    }

    return ret;
}
