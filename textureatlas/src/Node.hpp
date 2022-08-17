#ifndef __NODE_HPP__
#define __NODE_HPP__

#include "Rect.hpp"
#include <vector>

struct Node
{
    Node( const Rect& rect ) : rect( rect ), oldRect( rect ) { child[0] = nullptr; child[1] = nullptr; }
    ~Node() { delete child[0]; delete child[1]; }

    Node* Insert( const Rect& area, int align );
    Node* InsertPadded( const BRect& img, int align, int edges );
    void RemoveChildren();
    bool AssetFits( const std::vector<BRect>&, int edges, int align );

    Node* child[2];
    Rect rect;
    Rect oldRect;
    BRect imgData;
};

#endif
