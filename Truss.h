#pragma once

#include <set>
#include <algorithm>

#include "GeneticItem.h"
#include "Node.h"

struct Truss : GeneticItem<Truss>
{
public:
    struct Member
    {
        Member()
            : force( 0.0 ), known( false )
        {
        }

        Newton          force;
        NodeIterator    nodeA;
        NodeIterator    nodeB;
        bool            known;
        double          thickness;
    };
    typedef std::vector<Member> Members;

    struct Safety
    {
        Newton          maxForce;
        bool            tension;

        NodeIterator    nodeA;
        NodeIterator    nodeB;

        double          forceProportion;

        double          thickness;
    };
    typedef std::vector<Safety> Safeties;

    static constexpr double		MAX_TRUSS_LENGTH = 465.0;
    static constexpr double		MAX_MEMBER_LENGTH = 150.0;
    static constexpr Newton		MAXIMUM_TENSION = 250.0;
    static constexpr Newton		MAXIMUM_COMPRESSION( double thickness, double length )
    {
        return (740000.0 / (length * length)) * (thickness > 1.1 ? (thickness > 2.1 ? 26.0 : 8.0) : 1.0);
    }
	static const unsigned int	MAX_THICKNESS = 6; // Maximum thickness of sum of members at a node.
public:   
    Truss()
        : memberCount( 0 ), thicknessSum( 0.0 )
    {
    }
    Truss( const Truss& truss )
        : memberCount( 0 ), thicknessSum( 0.0 )
    {
        copy( truss.nodes, truss.nodes.begin(), truss.nodes.end() );
    }
    Truss&  operator =( const Truss& truss )
    {
        memberCount = 0;
        thicknessSum = 0.0;
        nodes.clear();

        copy( truss.nodes, truss.nodes.begin(), truss.nodes.end() );
        
        return *this;
    }

    void            create( const Truss& a, const Truss& b, bool side );

    double          fitness();

    void            connect( NodeIterator a, NodeIterator b, double thickness );
    void            disconnect( NodeIterator a, NodeIterator b );

    NodeIterator    findMiddle() const;

    Safeties        calculateSafeties( NodeIterator middle );

    NodeSet         nodes;
    int             memberCount;
    double          thicknessSum;
protected:
    Members         calculateMembers( NodeIterator node, double magnitude );
    bool            calculateNodeMembers( Members& member, NodeIterator it, Force initial );

    int             determinancy()
    {
        return (int)(memberCount - ((nodes.size() * 2) - 3));
    }

    // The method will also return the iterator of the last added element
    NodeIterator    copy( const NodeSet& set, NodeIterator start, NodeIterator end );
};