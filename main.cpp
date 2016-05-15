#include "Genetic.h"
#include "Truss.h"
#include "Mutations.h"
#include "Random.h"

#include <iostream>
#include <fstream>
#include <time.h>

const unsigned int TIME = 1800; // Time in seconds to run for.
const unsigned int FAMILY_SIZE = 50000; // Normal family size is at 300. The larger values mean more randomness but slower running

GeneticAlgorithm<Truss> algorithm;

int main()
{
    // Create example truss
    Truss exa;
    {
        auto a = exa.nodes.insert( Node( -231.0, 0.0 ) ).first;
        auto b = exa.nodes.insert( Node( -112.5, 20.0 ) ).first;
        auto c = exa.nodes.insert( Node( -100.0, -20.0 ) ).first;
        auto d = exa.nodes.insert( Node( 0.0, -40.0 ) ).first;
        auto e = exa.nodes.insert( Node( 10.0, 20.0 ) ).first;
        auto f = exa.nodes.insert( Node( 100.0, -20.0 ) ).first;
        auto g = exa.nodes.insert( Node( 112.5, 20.0 ) ).first;
        auto h = exa.nodes.insert( Node( 231.0, 0.0 ) ).first;

        exa.connect( a, b, 1.0 );
        exa.connect( a, c, 1.0 );
        exa.connect( b, e, 1.0 );
        exa.connect( b, c, 1.0 );
        exa.connect( c, d, 1.0 );
        exa.connect( c, e, 1.0 );
        exa.connect( d, e, 1.0 );
        exa.connect( d, f, 1.0 );
        exa.connect( e, f, 1.0 );
        exa.connect( e, g, 1.0 );
        exa.connect( f, g, 1.0 );
        exa.connect( f, h, 1.0 );
        exa.connect( g, h, 1.0 );
    }

    Truss exb;
    {
        auto a = exb.nodes.insert( Node( -232.0, 0.0 ) ).first;
        auto b = exb.nodes.insert( Node( -160.0, -100.0 ) ).first;
        auto c = exb.nodes.insert( Node( -105.0, 0.0 ) ).first;
        auto d = exb.nodes.insert( Node( -0.0, -100.0 ) ).first;
        auto e = exb.nodes.insert( Node( 30.0, 0.0 ) ).first;
        auto f = exb.nodes.insert( Node( 130.0, -100.0 ) ).first;
        auto g = exb.nodes.insert( Node( 165.0, 0.0 ) ).first;
        auto h = exb.nodes.insert( Node( 232.0, 0.0 ) ).first;

        exb.connect( a, b, 1.0 );
        exb.connect( a, c, 1.0 );
        exb.connect( b, c, 1.0 );
        exb.connect( b, d, 1.0 );
        exb.connect( c, d, 1.0 );
        exb.connect( c, e, 1.0 );
        exb.connect( d, e, 1.0 );
        exb.connect( d, f, 1.0 );
        exb.connect( e, f, 1.0 );
        exb.connect( e, g, 1.0 );
        exb.connect( f, g, 1.0 );
        exb.connect( f, h, 1.0 );
        exb.connect( g, h, 1.0 );
    }

    // This is for mixed mode. Original (unmixed) mode uses algorithm.init( FAMILY_SIZE, exa );
	algorithm.init( FAMILY_SIZE / 2, exa, FAMILY_SIZE / 2, exb );

    Truss best;
    double bestFitness = 0;

    std::cout << "Choose a seeding value (any integer)" << std::endl;
    unsigned int seedVal;
    std::cin >> seedVal;

    Random::seed( seedVal );

    time_t start;
    time_t now;

    time( &start );
    time( &now );

    // Now process
    do
    {
        algorithm.process();

        auto item = algorithm.fittest();
        if( item.fitness > bestFitness )
        {
            best = item.item;
            bestFitness = item.fitness;

            std::cout << "New best fitness found: " << bestFitness << std::endl;
        }

        time( &now );
    } while( difftime( now, start ) < TIME );

    auto members = best.calculateSafeties( best.findMiddle() );
    double minimum = std::min_element( members.begin(), members.end(), []( const Truss::Safety& a, const Truss::Safety& b ){ return a.maxForce < b.maxForce; } )->maxForce;

    std::cout << "Application ended. Truss being written to file in the form of points on a cartesian plane and connection definitions." << std::endl;
    std::cout << "Final design can hold a maximum force of: " << minimum << " Newtons, expected." << std::endl;

    std::fstream file( "TrussDesign.txt", std::fstream::out | std::fstream::trunc );

    file << "Using seed of: " << seedVal << std::endl;

    unsigned int count = 0;
    for( auto i = best.nodes.begin(); i != best.nodes.end(); (++i), (++count) )
    {
        file << "Node " << count << ": " << i->x << ", " << i->y << std::endl;
    }

    for( auto i = members.begin(); i != members.end(); ++i )
    {
        int a = (int)std::distance( best.nodes.begin(), i->nodeA );
        int b = (int)std::distance( best.nodes.begin(), i->nodeB );
        // Member info:
        file << "Node " << a << " connected to Node " << b << " using " << i->thickness << " sticks." << std::endl;
        file << "\tProportion of force = " << i->forceProportion << " (" << (i->tension ? "tension)." : "compression).") << std::endl;
        file << "\tLength of member = " << distance( *(i->nodeA), *(i->nodeB) ) << "mm." << std::endl;
    }

    file << "Total span = " << distance( *best.nodes.begin(), *best.nodes.rbegin() ) << "mm." << std::endl;
    file << "And middle point at: " << distance( *best.nodes.begin(), *best.findMiddle() ) << "mm from left." << std::endl;
    file << "Total of " << best.nodes.size() << " nodes, " << best.memberCount << " members, and " << best.thicknessSum << " popsicle sticks." << std::endl;

    file.close();

    std::cout << "Press any key to continue" << std::endl;

    std::cin.ignore();
    std::cin.get();
}