#pragma once

template <typename CRTP>
class GeneticItem abstract
{
public:
    typedef void Mutation( CRTP* );
public:
    GeneticItem()
    {}

    virtual void    create( const CRTP& a, const CRTP& b, bool side ) = 0;
    virtual double  fitness() = 0;
};