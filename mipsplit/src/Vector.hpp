#ifndef __DARKRL__VECTOR_HPP__
#define __DARKRL__VECTOR_HPP__

template<class T>
struct Vector2
{
    Vector2() : x( 0 ), y( 0 ) {}
    Vector2( T _x, T _y ) : x( _x ), y( _y ) {}

    bool operator==( const Vector2<T>& rhs ) const { return x == rhs.x && y == rhs.y; }
    bool operator!=( const Vector2<T>& rhs ) const { return !( *this == rhs ); }

    T x, y;
};

template<class T>
Vector2<T> operator+( const Vector2<T>& lhs, const Vector2<T>& rhs )
{
    return Vector2<T>( lhs.x + rhs.x, lhs.y + rhs.y );
}

template<class T>
Vector2<T> operator-( const Vector2<T>& lhs, const Vector2<T>& rhs )
{
    return Vector2<T>( lhs.x - rhs.x, lhs.y - rhs.y );
}

template<class T>
Vector2<T> operator*( const Vector2<T>& lhs, const float& rhs )
{
    return Vector2<T>( lhs.x * rhs, lhs.y * rhs );
}


typedef Vector2<int> v2i;
typedef Vector2<float> v2f;

#endif
