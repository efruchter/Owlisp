#pragma once

#include <string>
#include <assert.h>
#include <vector>
#include "Containers.h"

using namespace std;

const string ExpStart = "[";
const string ExpEnd = "]";
const string StrLit = "\"";
const string WhitespaceTokens[] = { " ", "\t", "\n", "\r" };

typedef OArray<string> TokenList;

int CountOf( const string& Input, char Token ) {
    int Count = 0;
    for ( char C : Input ) {
        if ( C == Token )
            Count++;
    }
    return Count;
}

int IndexOf( const string& Input, char Token ) {
    int Index = -1;
    for ( char C : Input ) {
        Index++;
        if ( C == Token )
            return Index;
    }
    return -1;
}

bool Contains( const string& Input, char Token ) {
    return IndexOf( Input, Token ) != -1;
}

bool IsWhiteSpace( const string& Token ) {
    for ( const auto& WS : WhitespaceTokens ) {
        if ( Token == WS )
            return true;
    }
    return false;
}

TokenList Tokenize( const string& Input ) {
    TokenList Tokens;
    string Token;
    bool WithinStrLiteral = false;
    for ( char RawChar : Input ) {
        const string Char{ RawChar };
        if ( !WithinStrLiteral && Char == StrLit ) {
            WithinStrLiteral = true;
            Token += Char;
        } else if ( WithinStrLiteral && Char == StrLit ) {
            WithinStrLiteral = false;
            Token += Char;
            Tokens.Add( Token );
            Token = "";
        } else if ( WithinStrLiteral ) {
            Token += Char;
        } else if ( Char == ExpStart ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( Token );
                Token = "";
            }
            Token += Char;
            Tokens.Add( Token );
            Token = "";
        } else if ( Char == ExpEnd ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( Token );
                Token = "";
            }
            Token += Char;
            Tokens.Add( Token );
            Token = "";
        } else if ( IsWhiteSpace( Char ) ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( Token );
                Token = "";
            }
        } else {
            Token += Char;
        }
    }
    return Tokens;
}

string TrimEnclosingQuotes( const string& In ) {
    if ( In.size() >= 2 && In[ 0 ] == StrLit[ 0 ] && In[ In.size() - 1 ] == StrLit[ 0 ] ) {
        return In.substr( 1, In.size() - 1 );
    }

    return In;
}
