#include "Mutations.h"
#include "Random.h"

#include <algorithm>

const unsigned int MAX_PASSES = 12;

void    addNode( Truss* truss )
{
    // Find a node with at most 4 connections
    std::vector<NodeIterator> potentials;
    for( auto i = truss->nodes.begin(); i != truss->nodes.end(); ++i )
    {
        if( i->connected.size() < 4 )
            potentials.push_back( i );
    }

    if( potentials.size() == 0 )
        return;

    NodeIterator nodeA = potentials[Random::gen( (unsigned int)potentials.size() )];
    NodeIterator nodeB = nodeA->connected[Random::gen( (unsigned int)nodeA->connected.size() )].node;

    // Now make a new node and insert it
    Node newNode;

    do
    {
        double angle = atan( (nodeA->x - nodeB->x) / (nodeA->y - nodeB->y) ) + Random::normalGen( 0.0, 0.1 );
        double dist = Random::normalGen( 70.0, 60.0 );

        // And extend it from midpoint away from the centre
        double midX = (nodeA->x + nodeB->x) / 2.0;
        double midY = (nodeA->y + nodeB->y) / 2.0;

        newNode.x = midX + copysign( (dist * cos( angle )), midX );
        newNode.y = midY + copysign( (dist * sin( angle )), midY );

        // ALSO continue if we find that adding this node makes the truss too large
        if( distance( newNode, *truss->nodes.begin() ) > (Truss::MAX_TRUSS_LENGTH + 5.0) || distance( newNode, *std::prev( truss->nodes.end() )) > (Truss::MAX_TRUSS_LENGTH + 5.0) )
            continue;

    } while( distance( newNode, *nodeA ) > Truss::MAX_MEMBER_LENGTH && distance( newNode, *nodeB ) > Truss::MAX_MEMBER_LENGTH );
    auto it = truss->nodes.insert( newNode );

    truss->connect( it.first, nodeA, 1.0 );
    truss->connect( it.first, nodeB, 1.0 );

}
void    removeNode( Truss* truss )
{
    // Find a suitable join. This would be one with only two joints.
    std::vector<NodeIterator> potentials;
    for( auto i = std::next( truss->nodes.begin() ); i != std::prev( truss->nodes.end() ); ++i )
    {
        if( i->connected.size() == 2 )
            potentials.push_back( i );
    }

    if( potentials.size() == 0 )
        return;

    NodeIterator it = potentials[Random::gen( (unsigned int)potentials.size() )];

    if( it == truss->findMiddle() )
        return;

    // Disconnect, which will erase the node as well
    truss->disconnect( it, it->connected[1].node );
    truss->disconnect( it, it->connected[0].node );
}
void    moveNode( Truss* truss )
{
    // Find a node at random
    auto it = std::next( truss->nodes.begin(), Random::gen( (unsigned int)truss->nodes.size()-1) );

    // Record it
    Node n;
    n.connected = it->connected;

    bool isCentre = (it == truss->findMiddle());

    // Now move it around
    int tries = 20;

retry:
    tries--;
    if( tries == 0 ) 
        return;

    n.x = it->x + Random::normalGen( 0.0, 15 );
    n.y = it->y + Random::normalGen( 0.0, 15 );

    for( auto i = n.connected.begin(); i != n.connected.end(); ++i )
    {
        if( distance( n, *i->node ) > Truss::MAX_MEMBER_LENGTH )
            goto retry;
    }

    if( isCentre && !(n.x > -5.0 && n.x < 5.0 && n.y < 0.0) )
        goto retry;

    // End of retry block

    // Now erase and re-insert
    auto newIt = truss->nodes.insert( n );

    if( newIt.second == false )
        return;

    // And re-do the members
    for( auto i = n.connected.begin(); i != n.connected.end(); ++i )
    {
        std::find( (i->node)->connected.begin(), (i->node)->connected.end(), it )->node = newIt.first;
    }

    truss->nodes.erase( it );
}
void    thicken( Truss* truss )
{
    // We can either add more sticks to a member, or remove some
    unsigned int mode = Random::gen( 2 );

    if( mode == 1 )
    {
        auto members = truss->calculateSafeties( truss->findMiddle() );
        auto member = std::min_element( members.begin(), members.end(), []( const Truss::Safety& a, const Truss::Safety& b ){ return a.maxForce < b.maxForce; } );

        if( member->tension == false && truss->thicknessSum < 20.6 )
        {
            // Check that this member has 5 or less sticks going through
            int aThickness = 0.0;
            int bThickness = 0.0;
            for( auto i = member->nodeA->connected.begin(); i != member->nodeA->connected.end(); ++i )
            {
                aThickness += (int)(i->thickness);
            }
            for( auto i = member->nodeB->connected.begin(); i != member->nodeB->connected.end(); ++i )
            {
                bThickness += (int)(i->thickness);
            }

            if( aThickness >= Truss::MAX_THICKNESS || bThickness >= Truss::MAX_THICKNESS)
                return;

            auto aConnection = std::find( member->nodeA->connected.begin(), member->nodeA->connected.end(), member->nodeB );
            auto bConnection = std::find( member->nodeB->connected.begin(), member->nodeB->connected.end(), member->nodeA );

            if( aConnection->thickness < 1.1 && truss->thicknessSum < 20.1 )
            {
                aConnection->thickness = 2.0;
                bConnection->thickness = 2.0;
                truss->thicknessSum += 1.0;
            }
            else if( aConnection->thickness < 2.1 && aConnection->thickness > 1.1 && truss->thicknessSum < 20.6 )
            {
                aConnection->thickness = 2.5;
                bConnection->thickness = 2.5;
                truss->thicknessSum += (2.5 - aConnection->thickness);
            }
        }
    }
    else
    {
        auto members = truss->calculateSafeties( truss->findMiddle() );
        Newton minMemberForce = std::min_element( members.begin(), members.end(), []( const Truss::Safety& a, const Truss::Safety& b ){ return a.maxForce < b.maxForce; } )->maxForce;

        std::vector<std::pair<NodeIterator, NodeIterator>> potentials;

        for( auto i = truss->nodes.begin(); i != truss->nodes.end(); ++i )
        {
            for( auto j = i->connected.begin(); j != i->connected.end(); ++j )
            {
                if( *j->node < *i || j->thickness < 1.1 )
                    continue;

                potentials.push_back( { i, j->node } );
            }
        }

        if( potentials.size() == 0 )
            return;

        auto selection = potentials[Random::gen( (unsigned int)potentials.size() )];

        Newton maxForce =   std::find_if( members.begin(), members.end(), 
                                [selection]( const Truss::Safety& member )
                                { 
                                    return (selection.first == member.nodeA && selection.second == member.nodeB) || (selection.first == member.nodeB && selection.second == member.nodeA);
                                } 
                            )->maxForce;

        if( maxForce > minMemberForce * 0.125 )
            return;

        auto aConnection = std::find( selection.first->connected.begin(), selection.first->connected.end(), selection.second );
        auto bConnection = std::find( selection.second->connected.begin(), selection.second->connected.end(), selection.first );

        truss->thicknessSum += (1.0 - aConnection->thickness);

        aConnection->thickness = 1.0;
        bConnection->thickness = 1.0;
    }
}

Truss::Mutation*   selectMutation()
{
    int chance = Random::gen( 5 );

    if( chance == 0 )
        return addNode;
    else if( chance == 1 )
        return removeNode;
    else if( chance == 2 )
        return thicken;
    else
        return moveNode;
}