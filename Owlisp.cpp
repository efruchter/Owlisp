#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <functional>
#include <sstream>

using namespace std;

const string ExpStart = "[";
const string ExpEnd = "]";
const string StrLit = "\"";
const vector<string> WhitespaceTokens = { " ", "\t", "\n", "\r" };

struct OExpr;
struct OAtom;
struct OMachine;
struct OIntrinsic;

typedef unsigned int uint;
typedef vector<OExpr*> OExprList;
typedef vector<OIntrinsic*> OIntrinsics;

enum class OExprType {
    Func,
    Expr,
    Data
};

struct OIntrinsic {
    string Token;
    function<OExpr* ( OExpr* )> Function;
};

struct OAtom {
    string Token;
};

struct OExpr {
    OExprType Type;
    OAtom Atom;
    OExprList Children;

    OExpr( OExprType Type ) : Type( Type ) {

    }
};

struct OMachine {
    OIntrinsics Intrinsics;
    OExprList GlobalExpressions;
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
            if ( Token.size() > 0 ) {
                Tokens.push_back( Token );
                Token = "";
            }
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

OExpr* ToOExpr_SingleNoEval( vector<string>& Tokens ) {
    OExpr* Expr = new OExpr{ OExprType::Expr };

    for ( uint i = 1; i < Tokens.size() - 1; i++ ) {
        OExpr* Atom = new OExpr{ OExprType::Data };
        Atom->Atom.Token = Tokens[ i ];
        Expr->Children.push_back( Atom );
    }

    return Expr;
}

void BuildIntrinsics( OMachine& Machine ) {
    { // print
        const string Token_Print = "print";
        OIntrinsic* Intrinsic = new OIntrinsic{};
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print]( const OExpr* Expr ) {
            assert( Expr->Children.size() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( uint i = 1; i < Expr->Children.size(); i++ ) {
                cout << Expr->Children[ i ]->Atom.Token.substr( 1, Expr->Children[ i ]->Atom.Token.size() - 2 );
            }
            return new OExpr{ OExprType::Data };
        };
        Machine.Intrinsics.push_back( Intrinsic );
    }
    { // print
        const string Token_Print = "println";
        OIntrinsic* Intrinsic = new OIntrinsic{};
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print]( const OExpr* Expr ) {
            assert( Expr->Children.size() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( uint i = 1; i < Expr->Children.size(); i++ ) {
                cout << Expr->Children[ i ]->Atom.Token.substr( 1, Expr->Children[ i ]->Atom.Token.size() - 2 ) << endl;
            }
            return new OExpr{ OExprType::Data };
        };
        Machine.Intrinsics.push_back( Intrinsic );
    }
    { // +
        const string Token_Addition = "+";
        OIntrinsic* Intrinsic = new OIntrinsic{};
        Intrinsic->Token = Token_Addition;
        Intrinsic->Function = [Token_Addition]( const OExpr* Expr ) {
            assert( Expr->Children.size() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Addition );
            int sum = 0;
            for ( uint i = 1; i < Expr->Children.size(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                int a = 0;
                Stream >> a;
                sum += a;
            }
            OExpr* Result = new OExpr{ OExprType::Data };
            stringstream SS;
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine.Intrinsics.push_back( Intrinsic );
    }
}

const OIntrinsic* FindIntrinsic( const OMachine* Machine, const OExpr* Expr )     {
    for ( const OIntrinsic* const Intrinsic : Machine->Intrinsics ) {
        if ( Intrinsic->Token == Expr->Children[ 0 ]->Atom.Token ) {
            return Intrinsic;
        }
    }
    return nullptr;
}

const OIntrinsic* FindFunc( const OMachine* Machine, const OExpr* Expr ) {
    for ( const OIntrinsic* const Intrinsic : Machine->Intrinsics ) {
        if ( Intrinsic->Token == Expr->Children[ 0 ]->Atom.Token ) {
            return Intrinsic;
        }
    }
    return nullptr;
}

int main() {
    OMachine Machine;
    BuildIntrinsics( Machine );

    const string ToParse1 =
        "  [println \"Hello World!\"]  ";

    const string ToParse2 =
        " [+ 1 2 3] ";

    const string ToParse3 =
        " [println [+ 1 2]]";

    auto Tokens = Tokenize( ToParse2 );

    OExpr* PrintExpr = ToOExpr_SingleNoEval( Tokens );

    auto Func = FindIntrinsic( &Machine, PrintExpr );
    auto Res = Func->Function( PrintExpr );

    cout << Res->Atom.Token << endl;

    //for ( const auto& Token : Tokens ) {
    //    cout << Token << endl;
    //}
}

