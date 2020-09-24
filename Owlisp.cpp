#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>

using namespace std;

const string ExpStart = "[";
const string ExpEnd = "]";
const string StrLit = "\"";
const vector<string> WhitespaceTokens = { " ", "\t", "\n" };

typedef unsigned int uint;

struct OAtom {
    string Data;
};

struct OExpr {
    OAtom Atom;
    vector<unique_ptr<OExpr>> Children;
};

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

vector<string> Tokenize( const string& Input ) {
    vector<string> Tokens;
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
            Tokens.push_back( Token );
            Token = "";
        } else if ( WithinStrLiteral ) {
            Token += Char;
        } else if ( Char == ExpStart ) {
            Token += Char;
            Tokens.push_back( Token );
            Token = "";
        } else if ( Char == ExpEnd ) {
            if ( Token.size() > 0 ) {
                Tokens.push_back( Token );
                Token = "";
            }
            Token += Char;
            Tokens.push_back( Token );
            Token = "";
        } else if ( IsWhiteSpace( Char ) ) {
            if ( Token.size() > 0 ) {
                Tokens.push_back( Token );
                Token = "";
            }
        } else {
            Token += Char;
        }
    }

    return Tokens;
}

int main() {
    const string ToParse = "[print \"hullo\"]" "[+ 1 [+ 2 4]]";
    auto Tokens = Tokenize( ToParse );

    for ( const auto& Token : Tokens ) {
        cout << Token << endl;
    }
}
