#pragma once

#include <vector>

template <typename T>
struct OArray {
    std::vector<T> Arr;

    T& operator[]( const int Index ) {
        return Arr[ Index ];
    }

    const T& operator[]( const int Index ) const {
        return Arr[ Index ];
    }

    T& Get( const int index ) {
        return Arr[ index ];
    }

    const T& Get( const int index ) const {
        return Arr[ index ];
    }

    void Add( T V ) {
        Arr.push_back( V );
    }

    void PushStack( T V ) {
        Arr.push_back( V );
    }

    T PopStack() {
        return Arr.pop_back();
    }

    int Length() const {
        return static_cast<int>( Arr.size() );
    }

    template<typename F>
    void SwapOrSet( const T& t, F Predicate ) {
        for ( int i = 0; i < Length(); i++ ) {
            if ( Predicate( Get( i ) ) ) {
                Get( i ) = t;
                return;
            }
        }

        Add( t );
    }
};
