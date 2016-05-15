#pragma once

namespace Random
{
    void            seed( unsigned int s );

    // Generates a number from (and including) 0 up to (but not including) max, or [0 -> max)
    unsigned int    gen( unsigned int max );
    double          normalGen( double mean, double sd );
}