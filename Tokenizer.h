#pragma once

#include "Containers.h"

#include <string>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

const string ExpStart = "(";
const string ExpEnd = ")";
const string StrLit = "`";
const string WhitespaceTokens[] = { " ", "\t", "\n", "\r" };


struct OToken {
    int Line{};
    int Indent{};
    string Token{};
};

OToken Make_OToken(const int Line, const int Indent, const string& Str ) {
    return { Line, Indent, Str };
}

typedef OArray<OToken> TokenList;

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

template <typename T>
T ParseTokenToPrimitive( const string& Token ) {
    T Out{};
	stringstream SS;
	SS << Token;
    SS >> Out;
    return Out;
}

TokenList Tokenize( const string& Input ) {
    TokenList Tokens{};
    string Token{};
    bool WithinStrLiteral = false;
    int Line = 1;
    int Indent = 0;

    for ( char RawChar : Input ) {
        const string Char{ RawChar };
        if ( !WithinStrLiteral && Char == StrLit ) {
            WithinStrLiteral = true;
            Token += Char;
        } else if ( WithinStrLiteral && Char == StrLit ) {
            WithinStrLiteral = false;
            Token += Char;
            Tokens.Add( OToken{ Line, Indent, Token } );
            Token = "";
        } else if ( WithinStrLiteral ) {
            Token += Char;
        } else if ( Char == ExpStart ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( OToken{ Line, Indent, Token } );
                Token = "";
            }
            Token += Char;
            Tokens.Add( OToken{ Line, Indent, Token } );
            Token = "";
        } else if ( Char == ExpEnd ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( OToken{ Line, Indent, Token } );
                Token = "";
            }
            Token += Char;
            Tokens.Add( OToken{ Line, Indent, Token } );
            Token = "";
        } else if ( IsWhiteSpace( Char ) ) {
            if ( Token.size() > 0 ) {
                Tokens.Add( OToken{ Line, Indent, Token } );
                Token = "";
            }
        } else {
            Token += Char;
        }

        if ( Char == "\n" ) {
            Line++;
            Indent = 0;
        } else {
            Indent++;
        }
    }

    #if PRINT_TOKENS
    for ( int i = 0; i < Tokens.Length(); i++ ) {
        std::cout << Tokens[ i ] << std::endl;
    }
    #endif

    return Tokens;
}

string TrimEnclosingQuotes( const string& In ) {
    if ( In.size() >= 2 && In[ 0 ] == StrLit[ 0 ] && In[ In.size() - 1 ] == StrLit[ 0 ] ) {
        return In.substr( 1, In.size() - 2 );
    }
    return In;
}

void StrReplaceAll( string& str, const string& from, const string& to ) {
    if ( from.empty() )
        return;
    size_t start_pos = 0;
    while ( ( start_pos = str.find( from, start_pos ) ) != string::npos ) {
        str.replace( start_pos, from.length(), to );
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

string FilterRawStringForPrinting( const string& from ) {
    string Out = TrimEnclosingQuotes( from );
    StrReplaceAll( Out, "\\n", "\n" );
    return Out;
}
