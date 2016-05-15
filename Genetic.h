#pragma once

#include <vector>
#include <algorithm>
#include <numeric>

#include "Random.h"
#include "GeneticItem.h"

template <typename CRTP>
class GeneticAlgorithm
{
public:
    typedef double Fitness;

    struct Item
    {
        Item()
        {
        }
        Item( CRTP i, Fitness f )
            : item( i ), fitness( f )
        {
        }
        CRTP                    item;
        Fitness                 fitness;
    };

    typedef std::vector<std::pair<CRTP*, CRTP*>> GeneticPairs;
public:
    std::vector<Item>       family;

    void				init( int familySize, CRTP& initial )
    {
		_familySize = familySize;

		double fitness = initial.fitness();

		if (isinf(fitness))
			throw std::runtime_error("Cannot start a genetic algorithm with an entirely defect population!");

		family.resize(familySize, { initial, fitness } );
    }
    void				init( int copyA, CRTP& a, int copyB, CRTP& b )
    {
        _familySize = copyA + copyB;

        double aFitness = a.fitness();
        double bFitness = b.fitness();

        if( isinf( aFitness ) || isinf( aFitness ) )
            throw std::runtime_error( "Cannot start a genetic algorithm with a defect population!" );

        family.resize( copyA, { a, aFitness } );
        family.resize( copyB, { b, bFitness } );
    }
    void				process()
    {
        GeneticPairs pairs;
        pairs = std::move( selection() );
        std::vector<Item> newFamily = recombination( pairs );

        family.clear();
        std::swap( family, newFamily );
        
        pairs.clear();

        mutate();
    }

    Item&				fittest()
    {
        return *std::max_element( family.begin(), family.end(), []( const Item& a, const Item& b ){ return a.fitness < b.fitness; } );
    }
protected:
    // Selects items and pairs them up
    GeneticPairs		selection()
    {
        // Choose fitness
        double average = std::accumulate( family.begin(), family.end(), 0.0, []( double init, const Item& item ){ return init + (double)item.fitness; } ) / family.size();

        if( fabs( average ) < DBL_EPSILON )
            throw std::runtime_error( "A fatal and impossible genetic defect has occured in the entire population." );

        for( auto i = family.begin(); i != family.end(); ++i )
        {
            i->fitness = i->fitness / average;
        }

        std::vector<CRTP*>   mates;

remate:
        for( unsigned int i = 0; i != family.size(); ++i )
        {
            for( unsigned int j = 0; j < (unsigned int)family[i].fitness; ++j )
                mates.push_back( &family[i].item );

            unsigned int chance = (unsigned int)(10000.0 * fmod( family[i].fitness, 1 ));
            if( Random::gen(10000) < chance )
                mates.push_back( &family[i].item );
        }

        if( mates.size() < _familySize / 2 )
            goto remate;

        GeneticPairs pairs;
        for( int i = (int)mates.size() - 1; i >= 1; i -= 2 )
        {
            unsigned int a = Random::gen( (unsigned int)mates.size() );
            unsigned int b = Random::gen( (unsigned int)mates.size() );

            while( a == b )
                b = Random::gen( (unsigned int)mates.size() );

            pairs.push_back( { mates[a], mates[b] } );
        }

        return pairs;
    }
    std::vector<Item>   recombination( const GeneticPairs& pairs )
    {
        std::vector<Item> newFamily;
        for( unsigned int i = 0; i < pairs.size(); ++i )
        {
            newFamily.push_back( { CRTP(), 0.0 } );
            newFamily[2 * i].item.create( *pairs[i].first, *pairs[i].second, true );

            newFamily.push_back( { CRTP(), 0.0 } );
            newFamily[(2 * i) + 1].item.create( *pairs[i].first, *pairs[i].second, false );
        }
        return newFamily;
    }
    void				mutate()
    {
        for( auto i = family.begin(); i != family.end(); ++i )
        {
            auto function = selectMutation();

            function( &(i->item) );

            i->fitness = i->item.fitness();

            if( isinf( i->fitness ) )
                i->fitness = 0.0;
        }
    }

	// Records original family size
	unsigned int		_familySize;
};