#include "Truss.h"
#include "Random.h"

#include <algorithm>
#include <map>

const unsigned int MAXIMUM_CALCULATION_PASSES = 21;
const double FITNESS_INTENSITY = 3.0; // This is the intensity mentioned in the workbook

void    calculateForce( const Force& t, Force& a, Force& b )
{
    a.mag = (b.y * (t.x * t.mag) - b.x * (t.y * t.mag)) / (b.x * a.y - b.y * a.x);
    b.mag = -(a.mag * a.x + (t.mag * t.x)) / b.x;
}
Vector  formVector( const Truss::Member& member, NodeIterator from )
{
    Vector v;
    if( member.nodeA == from )
    {
        v.x = member.nodeB->x - from->x;
        v.y = member.nodeB->y - from->y;
    }
    else
    {
        v.x = member.nodeA->x - from->x;
        v.y = member.nodeA->y - from->y;
    }
    return v;
}

// For now it works, though there are some potential improvements with a lot of work
void                Truss::create( const Truss& a, const Truss& b, bool side )
{
    // Determine the sides on which to swap
    const Truss* left;
    const Truss* right;
    if( side )
    {
        left = &a;
        right = &b;
    }
    else
    {
        left = &b;
        right = &a;
    }

    // Split down the middle
    auto leftMiddle = left->findMiddle();
    auto rightMiddle = right->findMiddle();

    if( leftMiddle == left->nodes.end() || rightMiddle == right->nodes.end() )
        throw std::exception( "Error: Trying to construct a truss using at least one invalid parent (the parent's middle node can't be found)" );

    // Ensure that the structures do not overlap
    while( std::prev( leftMiddle )->x >= rightMiddle->x )
        std::advance( leftMiddle, -1 );

    // Just to make sure
    nodes.clear();

    auto leftLast = copy( left->nodes, left->nodes.begin(), leftMiddle );
    copy( right->nodes, rightMiddle, right->nodes.end() );

    auto newMiddle = findMiddle();

    // Deleting nodes with 0 connections
    for( auto i = nodes.begin(); i != nodes.end(); )
    {
        if( i->connected.size() == 0 )
            i = nodes.erase( i );
        else
            ++i;
    }

    // Now we need to look for all missing connections and try to reconnect them.
    for( unsigned int counter = 0; counter < MAXIMUM_CALCULATION_PASSES; ++counter )
    {
        for( auto i = nodes.begin(); i != nodes.end(); )
        {
            if( determinancy() == 0 && i->connected.size() != 1 )
                break;

            // Check whether the node can be connected with an item on the other side
            if( fabs( i->x - rightMiddle->x ) > MAX_MEMBER_LENGTH )
            {
                ++i;
                continue;
            }

            // If they arent connected to any item 50mm closer to the centre, add another connection
            int connectedChance = 10;
            for( auto j = i->connected.begin(); j != i->connected.end(); ++j )
            {
                if( fabs( j->node->x - rightMiddle->x ) < 100.0 )
                {
                    connectedChance -= 5;
                    break;
                }
            }

            if( determinancy() > 0 )
                connectedChance = -1000;

            connectedChance -= ((int)i->connected.size() - 2)*2;

            if( i->connected.size() == 1 )
                connectedChance += 100;

            // There are two more factors to consider: Determinancy and number of connections
            if( connectedChance < -100 && i != newMiddle )
            {
                // Disconnect whole node which will automatically delete it
                for( int j = (int)i->connected.size() - 1; j >= 0; --j )
                    disconnect( i, i->connected[j].node );

                break;
            }

            if( connectedChance < 0 )
            {
                ++i;
                continue;
            }

            if( Random::gen(10) <= (unsigned int)connectedChance )
            {
                for( auto j = nodes.begin(); j != nodes.end(); ++j )
                {
                    // This will determine whether or not the node on this side is connecting to a node on the other side of the middle
                    bool sides = ((j->x >= rightMiddle->x) ^ (i->x >= rightMiddle->x)) || (Random::gen(30) <= (unsigned int)connectedChance );
                    if( distance( *j, *i ) <= MAX_MEMBER_LENGTH && (sides || j == newMiddle) &&
                        (std::find( j->connected.begin(), j->connected.end(), i ) == j->connected.end() && &*i != &*j) )
                    {
                        connect( i, j, 1.0 );
                        break;
                    }
                }
            }

            ++i;
        }

        if( determinancy() == 0 )
            break;
    }
}

double              Truss::fitness()
{
    if( determinancy() != 0 || thicknessSum > 23.0 || nodes.size() == 0 )
        return 0.0;

    // Check the dimensions
    double length = distance( *nodes.rbegin(), *nodes.begin() );
    double lowest = std::min_element( nodes.begin(), nodes.end(), []( const Node& a, const Node& b ){ return a.y < b.y; } )->y;

    if( !(length < (MAX_TRUSS_LENGTH) && length > (MAX_TRUSS_LENGTH - 10.0)  && lowest > -135.0) )
        return 0.0;

    // Give them 10 points for surviving this far. Congratulations!
    double fitness = 10.0;

    // Find the middle node that will directly carry the weight
    NodeIterator middle = findMiddle();

    if( middle == nodes.end() )
        return 0.0;

    auto members = calculateSafeties( findMiddle() );
    auto minElement = std::min_element( members.begin(), members.end(), []( const Truss::Safety& a, const Truss::Safety& b ){ return a.maxForce < b.maxForce; } );
   
    if( fabs( minElement->maxForce ) > DBL_EPSILON )
        fitness += pow( minElement->maxForce / 10.0, FITNESS_INTENSITY);

    //if( thicknessSum != 0 )
        //fitness += 4000.0 / thicknessSum;

    return fitness;
}

Truss::Safeties     Truss::calculateSafeties( NodeIterator middle )
{
    Members members = calculateMembers( middle, 1.0 );
    Safeties safeties( members.size() );
    
    unsigned int count = 0;
    for( auto i = members.begin(); i != members.end(); (++i), (++count) )
    {
        safeties[count].nodeA = i->nodeA;
        safeties[count].nodeB = i->nodeB;

        safeties[count].forceProportion = i->force;
        safeties[count].thickness = i->thickness;

        if( i->force < 0.0 )
        {
            safeties[count].maxForce = -MAXIMUM_COMPRESSION( i->thickness, distance( *i->nodeA, *i->nodeB ) ) / i->force;
            safeties[count].tension = false;
        }
        else
        {
            safeties[count].maxForce = MAXIMUM_TENSION / i->force;
            safeties[count].tension = true;
        }
    }

    return safeties;
}
Truss::Members      Truss::calculateMembers( NodeIterator node, double magnitude )
{
    Members members;
    members.reserve( memberCount );
    std::vector<bool>   completeNodes( nodes.size() );
    unsigned int complete = 0;

    // Account for the fact that though on paper the truss may be tilted, in real life the first and final point
    //  will be aligned orthogonal to gravity
    Vector tilt;
    tilt.x = nodes.rbegin()->x - nodes.begin()->x;
    tilt.y = nodes.rbegin()->y - nodes.begin()->y;
    
    double span = tilt.length();
    // Normalise
    tilt.x /= span;
    tilt.y /= span;

    // Note the swap of x and y, as we are finding the direction of gravity, which is orthogonal (and the negative sign)
    Vector gravity( tilt.y, -tilt.x );

    Vector leftDist = (*node) - (*nodes.begin()); // Vector subtraction
    Vector rightDist = (*nodes.rbegin()) - (*node); // Vector subtraction
    // Utilise the fact that the span will be equal to the distance between the two nodes
    // Also utilise the fact that we can determine moments using the dot product of the vector difference and the tilt
    //  (as the tilt is normal to gravity the force of gravity, i.e. magnitude, is preserved at its full value)
    Force leftNodeForce( -magnitude * dot( rightDist, tilt ) / span, gravity );
    Force rightNodeForce( -magnitude * dot( leftDist, tilt ) / span, gravity );
    Force middleForce( magnitude, gravity );
    

    // Fill every member with the correct item. As a result of this method,
    //  the members will always have nodeA as the smaller of the two nodes,
    //  and the elements are ordered by nodeA and then nodeB. (0, 1) < (0, 2) < (0, 3) < (1, 2)
    for( auto i = nodes.begin(); i != nodes.end(); ++i )
    {
        for( auto j = i->connected.begin(); j != i->connected.end(); ++j )
        {
            if( *j->node < *i )
                continue;

            Member m;
            m.force = 0.0;
            m.known = false;
            m.nodeA = i;
            m.nodeB = j->node;
            m.thickness = j->thickness;

            members.push_back( m );
        }
    }

    for( unsigned int i = 0; i < MAXIMUM_CALCULATION_PASSES && complete != nodes.size(); ++i )
    {
        // Every pass go through the nodes and look for items
        unsigned int k = 0;
        for( NodeIterator j = nodes.begin(); j != nodes.end(); ++j, ++k )
        {
            if( completeNodes[k] )
                continue;

            Force initial; // Initialised to 0

            if( j == node )
                initial += middleForce;
            else if( j == nodes.begin() )
                initial += leftNodeForce;
            else if( j == std::prev( nodes.end() ) )
                initial += rightNodeForce;

            bool success = calculateNodeMembers( members, j, initial );

            if( success )
            {
                completeNodes[k] = true;
                complete++;
            }
        }
    }

    if( complete != nodes.size() )
    {
        for( auto i = members.begin(); i != members.end(); ++i )
            i->force = DBL_MAX;
    }

    return members;
}
bool                Truss::calculateNodeMembers( Members& members, NodeIterator it, Force initial )
{
    // Find the members
    std::vector<std::pair<Members::iterator, Force>> unknowns;
    unknowns.reserve( 2 );

    Force resultant = initial;

    auto end = std::next( it );
    for( auto i = members.begin(); i != members.end() && i->nodeA != end; ++i )
    {
        if( i->nodeA == it || i->nodeB == it )
        {
            if( i->known == false )
                unknowns.push_back( { i, Force( 0.0, formVector( *i, it ) ) } );
            else
                resultant += Force( i->force, formVector( *i, it ) );
        }
    }

    if( unknowns.size() > 2 || resultant.mag == 0 )
        return false;

    if( unknowns.size() == 2 )
    {
        calculateForce( resultant, unknowns[0].second, unknowns[1].second );
        unknowns[0].first->force = unknowns[0].second.mag;
        unknowns[0].first->known = true;
        unknowns[1].first->force = unknowns[1].second.mag;
        unknowns[1].first->known = true;
    }
    else if( unknowns.size() == 1 )
    {
        unknowns[0].first->force = -resultant.mag;
    }
    
    return true;
}

NodeIterator        Truss::copy( const NodeSet& set, NodeIterator start, NodeIterator end )
{
    if( start == set.end() )
        return nodes.end();
    // Create one so we know the nodeset contains items
    Node n;
    n.x = start->x;
    n.y = start->y;

    auto ourStart = nodes.insert( n ).first;
    for( auto i = std::next( start ); i != end; ++i )
    {
        Node n;
        n.x = i->x;
        n.y = i->y;
        
        auto nodeIt = nodes.insert( n ).first;

        // Add members
        for( auto j = i->connected.begin(); j != i->connected.end(); ++j )
        {
            // Add it only if the previous node was already added
            if( *i < *(j->node) || *(j->node) < *start )
                continue;

            // Find the previous node
            auto otherNode = std::next( ourStart, std::distance( start, j->node ) );

            connect( nodeIt, otherNode, j->thickness );
        }
    }
    return std::prev( nodes.end() );
}
void                Truss::connect( NodeIterator a, NodeIterator b, double thickness )
{
    a->connected.push_back( { thickness, b } );
    b->connected.push_back( { thickness, a } );

    memberCount++;
    thicknessSum += thickness;// *(distance( *a, *b ) / 150.0);
}
void                Truss::disconnect( NodeIterator a, NodeIterator b )
{
    auto it = std::find_if( a->connected.begin(), a->connected.end(), [b]( Node::Connection& con ){ return con.node == b; } );
    double thickness = it->thickness;// *(distance( *a, *b ) / 150.0);
    a->connected.erase( it );
    b->connected.erase( std::find_if( b->connected.begin(), b->connected.end(), [a]( Node::Connection& con ){ return con.node == a; } ) );

    if( a->connected.size() == 0 )
        nodes.erase( a );
    if( b->connected.size() == 0 )
        nodes.erase( b );

    thicknessSum -= thickness;

    memberCount--;
}
NodeIterator        Truss::findMiddle() const
{
    auto i = nodes.begin();
    Vector tilt = *nodes.rbegin() - *nodes.begin();
    double span = tilt.length();

    tilt.x /= span;
    tilt.y /= span;

    for( i; i != nodes.end(); ++i )
    {
        double a = dot( (*i - *nodes.begin()), tilt );
        double b = dot( (*nodes.rbegin() - *i), tilt );

        if( fabs( a - b ) < 5.0 )
            return i;
    }
    return nodes.end();
}