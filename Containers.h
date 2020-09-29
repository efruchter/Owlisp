#pragma once

#include <vector>

template<typename T, typename E>
struct Return {
    T Out;
    bool ErrorOccured;
    E Error;

    Return( T& Out ) : Out( Out ), ErrorOccured( false ), Error() {};
    Return( T&& Out ) : Out( Out ), ErrorOccured( false ), Error() {};
    Return( T& Out, E& Error ) : Out( Out ), ErrorOccured( true ), Error( Error ) {};
    Return( T& Out, E&& Error ) : Out( Out ), ErrorOccured( true ), Error( Error ) {};
    Return( T&& Out, E& Error ) : Out( Out ), ErrorOccured( true ), Error( Error ) {};
    Return( T&& Out, E&& Error ) : Out( Out ), ErrorOccured( true ), Error( Error ) {};
};

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
    void SwapOrSet( const T& Element, F SwapIfTrue ) {
        for ( int i = 0; i < Length(); i++ ) {
            if ( SwapIfTrue( Get( i ) ) ) {
                Get( i ) = Element;
                return;
            }
        }

        Add( Element );
    }
};
