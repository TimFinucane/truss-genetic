#pragma once

#include <vector>
#include <set>
#include "Dimensional.h"

typedef double Newton;

struct Node : public Vector
{
    struct Connection
    {
        double thickness;
        std::set<Node>::iterator node;

        bool operator ==( std::set<Node>::iterator it )
        {
            return node == it;
        }
    };

    Node()
    {
    }
    Node( double xVal, double yVal )
        : Vector( xVal, yVal )
    {
    }

    mutable std::vector<Connection>  connected;

    bool operator >( const Node& node ) const
    {
        return x > node.x;
    }
    bool operator <( const Node& node ) const
    {
        return x < node.x;
    }
};
typedef std::set<Node>      NodeSet;
typedef NodeSet::iterator   NodeIterator;
