#ifndef __NODE_HPP__
#define __NODE_HPP__

#include "Rect.hpp"

struct Node
{
    Node( const Rect& rect ) : rect( rect ) { child[0] = NULL; child[1] = NULL; }
    ~Node() { delete child[0]; delete child[1]; }

    Node* Insert( Node* area, bool align );

    Node* child[2];
    Rect rect;
};

#endif
