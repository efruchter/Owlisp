#pragma once

#include <vector>
#include <assert.h>

template<typename T, typename E>
struct Return {
    T Out;
    bool ErrorOccured;
    E Error;

    bool IsGood() { return !ErrorOccured; };
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

    void Add( const T& V ) {
        Arr.push_back( V );
    }

    void Add( T& V ) {
        Arr.push_back( V );
    }

    void Add( T&& V ) {
        Arr.emplace_back( V );
    }

    void Add() {
        Add( {} );
    }

    void PushStack( T& V ) {
        Arr.push_back( V );
    }

    void PushStack( T&& V ) {
        Arr.emplace_back( V );
    }

    void PushStack() {
        PushStack( {} );
    }

    T PopStack() {
        assert( Arr.size() >= 1 );
        T Out = PeekStack();
        Arr.pop_back();
        return Out;
    }

    T& PeekStack() {
        assert( Arr.size() >= 1 );
        return Arr[ Arr.size() - 1 ];
    }

    T& PeekStackPrev() {
        assert( Arr.size() >= 2 );
        return Arr[ Arr.size() - 2 ];
    }

    T& Last() {
        assert( Arr.size() >= 1 );
        return Arr[ Arr.size() - 1 ];
    }

    int Length() const {
        return static_cast<int>( Arr.size() );
    }

    bool NonEmpty() const {
        return Length() > 0;
    }

    void Empty() {
        Arr.clear();
    }

    template<typename F>
    void SetOrAdd( const T& Element, F SetFirstIfTrue ) {
        for ( int i = 0; i < Length(); i++ ) {
            if ( SetFirstIfTrue( Get( i ) ) ) {
                Get( i ) = Element;
                return;
            }
        }

        Add( Element );
    }
};
