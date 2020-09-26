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
};
