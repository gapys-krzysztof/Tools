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

    if( area.w <= rect.w && area.h <= rect.h )
    {
        if( align )
        {
        }
        else
        {
            child[0] = new Node( Rect( rect.x + area.w, rect.y, rect.w - area.w, area.h ) );
            child[1] = new Node( Rect( rect.x, rect.y + area.h, rect.w, rect.h - area.h ) );
        }
        rect = Rect( rect.x, rect.y, area.w, area.h );
        ret = this;
    }

    return ret;
}
