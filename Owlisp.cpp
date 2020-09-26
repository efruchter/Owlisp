#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <functional>
#include <sstream>

#include "Containers.h"
#include "Tokenizer.h"

using namespace std;

struct OExpr;
struct OAtom;
struct OMachine;
struct OIntrinsic;

typedef unsigned int uint;

typedef shared_ptr<OExpr> OExprPtr;
typedef shared_ptr<OMachine> OMachinePtr;
typedef shared_ptr<OIntrinsic> OIntrinsicPtr;

typedef OArray<OExprPtr> OExprList;
typedef OArray<OIntrinsicPtr> OIntrinsics;
typedef function<OExprPtr( const OExprPtr )> IntrinsicFunction;

enum class OExprType {
    // A C++ Function.
    Intrinsic,
    // An evaluatable expression root
    Expr,
    // Data, cannot be expanded.
    Data,
    // List of Expr
    ExprList
};

struct OIntrinsic {
    OExprType Type;
    string Token;
    IntrinsicFunction Function;
};

struct OAtom {
    string Token;
};

struct OExpr {
    OExprType Type;
    OAtom Atom;
    OExprList Children;
};

struct OMachine {
    OIntrinsicPtr EmptyIntrinsic;
    OIntrinsics Intrinsics;
};

OExprPtr Make_OExprPtr_Empty() {
    OExprPtr Ptr = OExprPtr( new OExpr{} );
    assert( Ptr != nullptr );
    Ptr->Type = OExprType::Data;
    return Ptr;
}

OExprPtr Make_OExprPtr( const OExprType Type ) {
    OExprPtr Ptr = OExprPtr( new OExpr{} );
    assert( Ptr != nullptr );
    Ptr->Type = Type;
    return Ptr;
}

OExprPtr Make_OExprPtr_Data( const string& Token ) {
    OExprPtr AtomExpr = Make_OExprPtr( OExprType::Data );
    AtomExpr->Atom.Token = Token;
    return AtomExpr;
}

OIntrinsicPtr Make_OIntriniscPtr( const OExprType Type ) {
    OIntrinsicPtr Ptr = OIntrinsicPtr( new OIntrinsic{} );
    assert( Ptr != nullptr );
    Ptr->Type = Type;
    return Ptr;
}

OMachinePtr Make_OMachinePtr() {
    OMachinePtr Ptr = OMachinePtr( new OMachine{} );
    assert( Ptr != nullptr );
    return Ptr;
}

OExprPtr ToOExpr_SingleNoEval( const TokenList& Tokens ) {
    OExprPtr Expr = Make_OExprPtr( OExprType::Expr );

    for ( int i = 1; i < Tokens.Length() - 1; i++ ) {
        Expr->Children.Add( Make_OExprPtr_Data( Tokens[ i ] ) );
    }

    return Expr;
}

void BuildIntrinsics( OMachinePtr Machine ) {
    { // Empty Intrinsic
        Machine->EmptyIntrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Machine->EmptyIntrinsic->Function = []( const OExprPtr Expr ) {
            return Make_OExprPtr_Empty();
        };
    }

    { // print
        const string Token_Print = "print";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                cout << TrimEnclosingQuotes( Expr->Children[ i ]->Atom.Token );
            }
            return Make_OExprPtr_Empty();
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // println
        const string Token_Print = "println";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                cout << TrimEnclosingQuotes( Expr->Children[ i ]->Atom.Token ) << endl;
            }
            return Make_OExprPtr_Empty();
        };
        Machine->Intrinsics.Add( Intrinsic );
    }

    { // +
        const string Token_Addition = "+";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Addition;
        Intrinsic->Function = [Token_Addition]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Addition );
            int sum = 0;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                int a = 0;
                Stream >> a;
                sum += a;
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }

    { // -
        const string Token_Sub = "-";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Sub;
        Intrinsic->Function = [Token_Sub]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Sub );
            int sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                int a = 0;
                Stream >> a;
                if ( set ) {
                    sum -= a;
                } else {
                    set = true;
                    sum = a;
                }
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }

    { // * Multiplication
        const string Token_Mul = "*";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Mul;
        Intrinsic->Function = [Token_Mul]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Mul );
            float sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                float a = 0;
                Stream >> a;
                if ( set ) {
                    sum *= a;
                } else {
                    sum = a;
                    set = true;
                }
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }

    { // / Floating Point Division
        const string Token_Div = "/";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_Div;
        Intrinsic->Function = [Token_Div]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Div );
            float sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                float a = 0;
                Stream >> a;
                if ( set ) {
                    sum = sum / a;
                } else {
                    sum = a;
                    set = true;
                }
            }
            if ( !set ) {
                sum = 0;
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }

    { // // Integer Division
        const string Token_IDiv = "//";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
        Intrinsic->Token = Token_IDiv;
        Intrinsic->Function = [Token_IDiv]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_IDiv );
            int sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << Expr->Children[ i ]->Atom.Token;
                int a = 0;
                Stream >> a;
                if ( set ) {
                    sum = sum / a;
                } else {
                    sum = a;
                    set = true;
                }
            }
            if ( !set ) {
                sum = 0;
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = SS.str();
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
}

const OIntrinsicPtr FindIntrinsic( const OMachinePtr Machine, const OExprPtr Expr ) {
    for ( int i = 0; i < Machine->Intrinsics.Length(); i++ ) {
        if ( Machine->Intrinsics[ i ]->Token == Expr->Children[ 0 ]->Atom.Token ) {
            return Machine->Intrinsics[ i ];
        }
    }
    return Machine->EmptyIntrinsic;
}

const OIntrinsicPtr FindFunc( const OMachinePtr Machine, const OExprPtr Expr ) {
    for ( int i = 0; i < Machine->Intrinsics.Length(); i++ ) {
        if ( Machine->Intrinsics[ i ]->Token == Expr->Children[ 0 ]->Atom.Token ) {
            return Machine->Intrinsics[ i ];
        }
    }
    return nullptr;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens, int StartIndex, int EndIndex ) {
    if ( StartIndex == EndIndex ) {
        return Make_OExprPtr_Data( Tokens[ StartIndex ] );
    }

    if ( Tokens[ StartIndex ] != ExpStart
        && Tokens[ EndIndex ] != ExpEnd ) {
        OExprPtr Root = Make_OExprPtr( OExprType::Expr );

        for ( int i = StartIndex; i <= EndIndex; i++ ) {
            Root->Children.Add( Make_OExprPtr_Data( Tokens[ i ] ) );
        }

        return Root;
    }

    OExprPtr Root = Make_OExprPtr( OExprType::Expr );

    if ( StartIndex == 0 && EndIndex == Tokens.Length() - 1 ) {
        Root->Type = OExprType::ExprList;
    }

    int InitBracketIndex = 0;
    int OpenBracketCount = 0;
    for ( int i = StartIndex; i <= EndIndex; i++ ) {
        if ( Tokens[ i ] == ExpStart ) {
            if ( OpenBracketCount == 0 ) {
                InitBracketIndex = i;
            }
            OpenBracketCount++;
        } else if ( Tokens[ i ] == ExpEnd ) {
            OpenBracketCount--;
            if ( OpenBracketCount == 0 ) {
                Root->Children.Add( ConstructRootExpr( Tokens, InitBracketIndex + 1, i - 1 ) );
            }
        } else if ( OpenBracketCount == 0 ) {
            Root->Children.Add( Make_OExprPtr_Data( Tokens[ i ] ) );
        }
    }

    return Root;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens ) {
    return ConstructRootExpr( Tokens, 0, Tokens.Length() - 1 );
}

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr ) {
    if ( Expr->Type == OExprType::Data ) {
        return Expr;
    }

    if ( Expr->Type == OExprType::Expr ) {
        for ( int i = 1; i < Expr->Children.Length(); i++ ) {
            if ( Expr->Children[ i ]->Type == OExprType::Expr ) {
                Expr->Children[ i ]->Atom = EvalExpr( Machine, Expr->Children[ i ] )->Atom;
            }
        }

        OIntrinsicPtr Intrinsic = FindIntrinsic( Machine, Expr );
        if ( Intrinsic != Machine->EmptyIntrinsic ) {
            return Intrinsic->Function( Expr );
        }
    }

    return Make_OExprPtr_Empty();
}

void ResetMachine( OMachinePtr Machine) {
    *Machine = {};
    BuildIntrinsics( Machine );
}

int main() {
    OMachinePtr Machine = Make_OMachinePtr();
    ResetMachine( Machine );

    const string ToParse1 =
        "  [println \"Hello World!\"]  ";

    const string ToParse2 =
        " [ + 1 8 7 ] "; // 

    const string ToParse3 =
        " [println \"Aww yus, time to Eval..\"] "
        " [println [+ 1 8 [* 9 2]]] "; // 27

    const TokenList Tokens = Tokenize( ToParse3 );
    const OExprPtr Program = ConstructRootExpr( Tokens );

    assert( Program->Type == OExprType::ExprList );
    for ( int i = 0; i < Program->Children.Length(); i++ ) {
        EvalExpr( Machine, Program->Children[ i ] );
    }
}

