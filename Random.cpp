#include "Random.h"

#include <random>

using namespace Random;

std::default_random_engine  randomEngine;

void            Random::seed( unsigned int s )
{
    randomEngine.seed( s );
}
unsigned int    Random::gen( unsigned int max )
{
    std::uniform_int_distribution<int> dist( 0, max - 1 );

    return dist( randomEngine );
}
double          Random::normalGen( double mean, double sd )
{
    std::normal_distribution<double> dist( mean, sd );

    return dist( randomEngine );
}