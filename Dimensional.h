#pragma once

#include <math.h>

struct Vector
{
    Vector()
        : x( 0.0 ), y( 0.0 )
    {
    }
    Vector( double xVal, double yVal )
        : x( xVal ), y( yVal )
    {
    }
    double x;
    double y;

    double  length() const
    {
        return sqrt( x * x + y * y );
    }

    Vector      operator +( const Vector& vec ) const
    {
        return Vector( x + vec.x, y + vec.y );
    }
    Vector      operator -( const Vector& vec ) const
    {
        return Vector( x - vec.x, y - vec.y );
    }
    Vector&     operator +=( const Vector& vec )
    {
        x += vec.x;
        y += vec.y;

        return *this;
    }
    Vector&     operator -=( const Vector& vec )
    {
        x -= vec.x;
        y -= vec.y;

        return *this;
    }
};

inline double distance( const Vector& a, const Vector& b )
{
    Vector v( a.x - b.x, a.y - b.y );

    return v.length();
}

inline double dot( const Vector& a, const Vector& b )
{
    return a.x * b.x + a.y * b.y;
}

struct Force : public Vector
{
    Force()
        : mag( 0.0 )
    {
    }
    Force( double m, double xVal, double yVal )
        : Vector( xVal, yVal )
    {
        normalise();

        if( m >= 0.0 )
            mag = m;
        else
        {
            x = -x;
            y = -y;
            mag = -m;
        }

    }
    Force( double m, Vector v )
        : Vector( v )
    {
        normalise();

        if( m >= 0.0 )
            mag = m;
        else
        {
            x = -x;
            y = -y;
            mag = -m;
        }
    }

    double mag;

    // Overrides, but is not virtual, so dont upcast a force.
    double length()
    {
        return mag;
    }

    Force operator +( const Vector& vec )
    {
        Force f;
        f.normalise( convert() + vec );

        return f;
    }
    Force operator -( const Vector& vec )
    {
        Force f;
        f.normalise( convert() - vec );

        return f;
    }
    Force& operator +=( const Vector& vec )
    {
        normalise( convert() + vec );

        return *this;
    }
    Force& operator -=( const Vector& vec )
    {
        normalise( convert() - vec );

        return *this;
    }

    Force& operator +=( const Force& f )
    {
        normalise( { mag * x + f.mag * f.x, mag * y + f.mag * f.y } );

        return *this;
    }
    Force& operator -=( const Force& f )
    {
        normalise( { mag * x - f.mag * f.x, mag * y - f.mag * f.y } );

        return *this;
    }
private:
    inline void    normalise()
    {
        mag = Vector::length();
        if( fabs( mag ) < DBL_EPSILON )
            return;

        x = x / mag;
        y = y / mag;
    }
    inline void    normalise( const Vector& vector )
    {
        mag = vector.length();
        if( fabs( mag ) < DBL_EPSILON )
            return;

        x = vector.x / mag;
        y = vector.y / mag;
    }
    inline Vector  convert()
    {
        return { mag * x, mag * y };
    }
};