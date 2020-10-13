#define MANAGE_EXPR_MEM 1
#define PRINT_TOKENS 0
#define PRINT_EVAL 0

#include "Owlisp.h"
#include <iostream>
#include <math.h> 

int main( int argc, char* argv[] ) {
    OMachinePtr Machine = Make_OMachinePtr();
    ResetMachine( Machine );
    if ( argc > 1 ) {
        const string arg1{ argv[ 1 ] };
        if ( arg1 == "-i" ) {
            InterpreterLoop( Machine );
            return 0;
        }
        // Compile the file arg1
        auto InputRet = ReadFileIntoString( arg1 );
        if ( InputRet.ErrorOccured ) {
            std::cerr << InputRet.Error << std::endl;
            return 1;
        }
        const TokenList Tokens = Tokenize( InputRet.Out );
        const OExprPtr Program = ConstructRootExpr( Tokens );
        Execute( Machine, Program );
        return 0;
    }
    std::cerr << "Please use -1 for interpreter or a filename to run." << std::endl;
    return 1;
}

OExprPtr Make_OExprPtr_Empty() {
    OExprPtr Ptr = OExprPtr( new OExpr{} );
    assert( Ptr != nullptr );
    Ptr->Type = OExprType::Expr;
    return Ptr;
}

OExprPtr Make_OExprPtr( const OExprType Type ) {
    OExprPtr Ptr = OExprPtr( new OExpr{} );
    assert( Ptr != nullptr );
    Ptr->Type = Type;
    return Ptr;
}

OExprPtr Make_OExprPtr_Data( const OToken& Token ) {
    OExprPtr AtomExpr = Make_OExprPtr( OExprType::Data );
    AtomExpr->Atom.Token = Token;
    return AtomExpr;
}

OExprPtr Make_OExprPtr_Data( const OAtom& Atom, const string& Str ) {
    OExprPtr AtomExpr = Make_OExprPtr( OExprType::Data );
    AtomExpr->Atom.Token = Make_OToken( Atom, Str );
    return AtomExpr;
}

OExprPtr Make_OExprPtr_DataExprCap( bool StartCap ) {
    return Make_OExprPtr_Data( Make_OToken( StartCap ? ExpStart : ExpEnd ) );
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

OToken Make_OToken( const OAtom& Atom, const string& Str ) {
    return {Atom.Token.Line, Atom.Token.Indent, Str };
}

OToken Make_OToken( const string& Str ) {
    return { 0, 0, Str };
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
        Machine->EmptyIntrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Machine->EmptyIntrinsic->Function = []( const OExprPtr Expr ) {
            return Make_OExprPtr_Empty();
        };
    }
    { // exit
        const string Token_Exit = "exit";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Exit;
        Intrinsic->Function = [Token_Exit, Machine]( const OExprPtr Expr ) {
            Machine->ShouldExit = true;
            return Make_OExprPtr_Empty();
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // print
        const string Token_Print = "print";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token== Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                cout << FilterRawStringForPrinting( EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token);
            }
            return Make_OExprPtr_Empty();
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // println
        const string Token_Print = "println";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Print;
        Intrinsic->Function = [Token_Print, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token== Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                OExprPtr Result = EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute );
                cout << FilterRawStringForPrinting( Result->Atom.Token.Token) << endl;
            }
            if ( Expr->Children.Length() == 1 ) {
                cout << endl;
            }
            return Make_OExprPtr_Empty();
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // +
        const string Token_Addition = "+";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Addition;
        Intrinsic->Function = [Token_Addition, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token== Token_Addition );
            int sum = 0;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                sum += ParseTokenToPrimitive<int>( EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token);
            }
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sum;
            Result->Atom.Token = Make_OToken( Expr->Children[ 0 ]->Atom, SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // -
        const string Token_Sub = "-";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Sub;
        Intrinsic->Function = [Token_Sub, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token == Token_Sub );
            int sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
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
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // * Multiplication
        const string Token_Mul = "*";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Mul;
        Intrinsic->Function = [Token_Mul, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token == Token_Mul );
            float sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
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
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // sqrt
        const string Token_Sqrt = "sqrt";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Sqrt;
        Intrinsic->Function = [Token_Sqrt, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 2 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token == Token_Sqrt );
            stringstream Stream;
            Stream << EvalExpr( Machine, Expr->Children[ 1 ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
            float a = 0;
            Stream >> a;
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << sqrtf( a );
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // / Floating Point Division
        const string Token_Div = "/";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Div;
        Intrinsic->Function = [Token_Div, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token == Token_Div );
            float sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
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
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // // Integer Division
        const string Token_IDiv = "//";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_IDiv;
        Intrinsic->Function = [Token_IDiv, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token.Token == Token_IDiv );
            int sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
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
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // // Integer Modulo
        const string Token_IMod = "modi";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_IMod;
        Intrinsic->Function = [Token_IMod, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );

            stringstream SS{};
            SS << EvalExpr( Machine, Expr->Children[ 1 ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
            int I = 0;
            SS >> I;

            SS = {};
            SS << EvalExpr( Machine, Expr->Children[ 2 ], EEvalIntrinsicMode::Execute )->Atom.Token.Token;
            int M = 0;
            SS >> M;

            int ResultI = ( I % M );
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            SS = {};
            SS << ResultI;
            Result->Atom.Token = Make_OToken( SS.str() );
            return Result;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // Set
        const string Token_Set = "=";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Set;
        Intrinsic->Function = [Token_Set, Machine]( const OExprPtr Expr ) {
            const int KeyIndex = 1;
            assert( Expr->Children[ 0 ]->Atom.Token.Token== Token_Set );
            assert( Expr->Children.Length() == KeyIndex + 2 );
            OExprPtr NewExpr = Make_OExprPtr( OExprType::Expr );
            NewExpr->Children.Add( Expr->Children[ KeyIndex ] );
            NewExpr->Children.Add( EvalExpr( Machine, Expr->Children[ KeyIndex + 1 ], EEvalIntrinsicMode::Execute ) );
            Machine->Stack.PeekStack().SetOrAdd( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
                if ( ExistingExpr->Children.Length() == 2 ) {
                    if ( ExistingExpr->Children[ 0 ]->Atom.Token.Token== NewExpr->Children[ 0 ]->Atom.Token.Token) {
                        return true;
                    }
                }
                return false;
            } );

            return NewExpr;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // defunc
        const string Token_Defunc = TOKEN_DEFUNC;
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Defunc;
        Intrinsic->Function = [Token_Defunc, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() >= 3 ); // defunc FuncName (Param*) FuncBody
            assert( Expr->Children[ 0 ]->Atom.Token.Token== Token_Defunc );
            OExprPtr NewExpr = Make_OExprPtr( OExprType::ExprFunc );
            // Add FuncName node
            NewExpr->Children.Add( Expr->Children[ 1 ] ); // EvalExpr( Machine, Expr->Children[ 1 ], EEvalIntrinsicMode::Execute ) );
            for ( int i = 2; i < Expr->Children.Length() - 1; i++ ) {
                //Add Param nodes
                NewExpr->Children.Add( Expr->Children[ i ] ); // EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::NoExecute ) );
            }
            NewExpr->Children.Add( Expr->Children.Last() );// EvalExpr( Machine, Expr->Children.Last(), EEvalIntrinsicMode::NoExecute ) );
            Machine->Stack.PeekStack().SetOrAdd( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
                if ( ExistingExpr->Type == OExprType::ExprFunc ) {
                    if ( ExistingExpr->Children[ 0 ]->Atom.Token.Token== NewExpr->Children[ 0 ]->Atom.Token.Token) {
                        return true;
                    }
                }
                return false;
            } );

            return NewExpr;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // ? Pick branch
        const string Token_BranchPick = "?";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_BranchPick;
        Intrinsic->Function = [Token_BranchPick, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() >= 3 ); //
            auto Res = EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute );
            if ( Res->Atom.Token.Token == "0" ) {
                if ( Expr->Children.Length() > 3 ) {
                    return EvalExpr( Machine, Expr->Get( 3 ), EEvalIntrinsicMode::Execute );
                } else {
                    return Make_OExprPtr_Empty();
                }
            } else {
                return EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
            }
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // ==
        const string Token_Equality = "==";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Equality;
        Intrinsic->Function = [Token_Equality, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            OExprPtr LHS = EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute );
            OExprPtr RHS = EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
            return Make_OExprPtr_Data( Expr->Atom, TopAtom( LHS ).Token.Token == TopAtom( RHS ).Token.Token ? TOKEN_TRUE : TOKEN_FALSE );
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // <
        const string Token_LessThan = "<";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_LessThan;
        Intrinsic->Function = [Token_LessThan, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            OExprPtr LHS = EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute );
            OExprPtr RHS = EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
            return Make_OExprPtr_Data( Expr->Atom, ( CompareTo( TopAtom( LHS ).Token.Token, TopAtom( RHS ).Token.Token ) < 0 ) ? TOKEN_TRUE : TOKEN_FALSE );
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // >
        const string Token_GreaterThan = ">";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_GreaterThan;
        Intrinsic->Function = [Token_GreaterThan, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            OExprPtr LHS = EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute );
            OExprPtr RHS = EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
            return Make_OExprPtr_Data( Expr->Atom, ( CompareTo( TopAtom( LHS ).Token.Token, TopAtom( RHS ).Token.Token ) > 0 ) ? TOKEN_TRUE : TOKEN_FALSE );
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { //join
        const string Token_StrJoin = "strjoin";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_StrJoin;
        Intrinsic->Function = [Token_StrJoin, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            stringstream OutStream{};
            const string Delim = FilterRawStringForPrinting( TopAtom( EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute ) ).Token.Token );
            const auto Child = EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute, EEvalExprReturnMode::TopExpr );
            for ( int i = 0; i < Child->Children.Length(); i++ ) {
                OutStream << FilterRawStringForPrinting( TopAtom( EvalExpr( Machine, Child->Children[ i ], EEvalIntrinsicMode::Execute ) ).Token.Token );
                if ( i != ( Child->Children.Length() - 1 ) ) {
                    OutStream << Delim;
                }
            }
            OExprPtr RHS = EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
            return Make_OExprPtr_Data( Expr->Atom, OutStream.str() );
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // map
        const string Token_Map = "map";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Map;
        Intrinsic->Function = [Token_Map, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            // 0: Name, 1: mapfunc, 2: (array)
            // Goal, build 2-node tuples of each. A zip of func and data, then execute.
            if ( Expr->Get( 1 )->Children.Length() == 2 ) {
                Machine->Stack.PushStack();
                OExprPtr Func = Make_OExprPtr( OExprType::ExprFunc );
                const string MapFuncName = "_MapFunc";
                Func->Children.Add( Make_OExprPtr_Data(Expr->Atom, MapFuncName ) );
                Func->Children.Add( Expr->Get( 1 )->Children[ 0 ] );
                Func->Children.Add( Expr->Get( 1 )->Children[ 1 ] );
                OExprPtr Out = Make_OExprPtr( OExprType::Expr );
                for ( int i = 0; i < Expr->Get( 2 )->Children.Length(); i++ ) {
                    OExprPtr NamedFunc = Make_OExprPtr( OExprType::Expr );
                    NamedFunc->Children.Add( Make_OExprPtr_Data( Expr->Atom, MapFuncName ) );
                    NamedFunc->Children.Add( Expr->Get( 2 )->Children[ i ] );
                    Out->Children.Add( EvalNamedFunction( Machine, NamedFunc, Func, EEvalIntrinsicMode::Execute ) );
                }
                Machine->Stack.PopStack();
                return Out;
            } else {
                OExprPtr Out = Make_OExprPtr( OExprType::Expr );
                for ( int i = 0; i < Expr->Get( 2 )->Children.Length(); i++ ) {
                    OExprPtr Zip = Make_OExprPtr( OExprType::Expr );
                    Zip->Children.Add( Expr->Get( 1 ) );
                    Zip->Children.Add( Expr->Get( 2 )->Children[ i ] );
                    Out->Children.Add( EvalExpr( Machine, Zip, EEvalIntrinsicMode::Execute ) );
                }
                return Out;
            }
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // reduce
        const string Token_Reduce = "reduce";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Reduce;
        Intrinsic->Function = [Token_Reduce, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 3 );
            // 0: Name, 1: mapfunc, 2: (array)
            // Goal, build 2-node tuples of each. A zip of func and data, then execute.
            if ( Expr->Get( 1 )->Children.Length() == 3 ) {
                Machine->Stack.PushStack();
                OExprPtr Func = Make_OExprPtr( OExprType::ExprFunc );
                const string MapFuncName = "_MapFunc";
                Func->Children.Add( Make_OExprPtr_Data( Expr->Atom, MapFuncName ) );
                Func->Children.Add( Expr->Get( 1 )->Children[ 0 ] );
                Func->Children.Add( Expr->Get( 1 )->Children[ 1 ] );
                Func->Children.Add( Expr->Get( 1 )->Children[ 2 ] );
                OExprPtr Out = Expr->Get( 2 )->Children[ 0 ];
                for ( int i = 1; i < Expr->Get( 2 )->Children.Length(); i++ ) {
                    OExprPtr NamedFunc = Make_OExprPtr( OExprType::Expr );
                    NamedFunc->Children.Add( Make_OExprPtr_Data( Expr->Atom, MapFuncName ) );
                    NamedFunc->Children.Add( Out );
                    NamedFunc->Children.Add( Expr->Get( 2 )->Children[ i ] );
                    Out = EvalNamedFunction( Machine, NamedFunc, Func, EEvalIntrinsicMode::Execute );
                }
                Machine->Stack.PopStack();
                return Out;
            } else {
                OExprPtr Out = Expr->Get( 2 )->Children[ 0 ];
                for ( int i = 1; i < Expr->Get( 2 )->Children.Length(); i++ ) {
                    OExprPtr Zip = Make_OExprPtr( OExprType::Expr );
                    Zip->Children.Add( Expr->Get( 1 ) );
                    Zip->Children.Add( Out );
                    Zip->Children.Add( Expr->Get( 2 )->Children[ i ] );
                    Out = EvalExpr( Machine, Zip, EEvalIntrinsicMode::Execute );
                }
                return Out;
            }
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { // return
        const string Token_Return = "return";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Return;
        Intrinsic->Function = [Token_Return, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() >= 0 );
            OExprPtr Out = Make_OExprPtr( OExprType::Break );
            if ( Expr->Children.Length() == 2 ) {
                Out->Children.Add( EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute ) );
            }
            return Out;
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
    { //loop (loop (T1) (T2) (T3) .. (return T4))
        const string Token_Loop = "loop";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Loop;
        Intrinsic->Function = [Token_Loop, Machine]( const OExprPtr Expr ) {
            if ( Expr->Children.Length() <= 1 ) {
                return Make_OExprPtr_Empty();
            }
            while ( true ) {
                for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                    OExprPtr Out = EvalExpr( Machine, Expr->Get( i ), EEvalIntrinsicMode::Execute );
                    if ( Out->Type == OExprType::Break ) {
                        if ( Out->Children.Length() == 1 ) {
                            return Out->Get( 0 );
                        } else {
                            return Make_OExprPtr_Empty();
                        }
                    }
                }
            }
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
}

const OIntrinsicPtr FindIntrinsic( const OMachinePtr Machine, const OExprPtr Expr ) {
    for ( int i = 0; i < Machine->Intrinsics.Length(); i++ ) {
        if ( Machine->Intrinsics[ i ]->Token == TopAtom( Expr ).Token.Token ) {
            return Machine->Intrinsics[ i ];
        }
    }
    return Machine->EmptyIntrinsic;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens, int StartIndex, int EndIndex ) {
    if ( StartIndex == EndIndex ) {
        return Make_OExprPtr_Data( Tokens[ StartIndex ] );
    }
    // Check if no more expansion is needed.
    {
        bool IsLeaf = true;
        bool InLiteral = false;
        for ( int i = StartIndex; i <= EndIndex; i++ ) {
            if ( !InLiteral && Tokens[ i ].Token == StrLit ) {
                InLiteral = true;
            } else if ( InLiteral && Tokens[ i ].Token == StrLit ) {
                InLiteral = false;
            } else if ( !InLiteral && ( Tokens[ i ].Token == ExpStart || Tokens[ i ].Token == ExpEnd ) ) {
                IsLeaf = false;
                break;
            }
        }
        if ( IsLeaf ) {
            OExprPtr Root = Make_OExprPtr( OExprType::Expr );

            for ( int i = StartIndex; i <= EndIndex; i++ ) {
                Root->Children.Add( Make_OExprPtr_Data( Tokens[ i ] ) );
            }

            return Root;
        }
    }
    // Needs more expansion
    OExprPtr Root = Make_OExprPtr( OExprType::Expr );
    int InitBracketIndex = 0;
    int OpenBracketCount = 0;
    for ( int i = StartIndex; i <= EndIndex; i++ ) {
        if ( Tokens[ i ].Token == ExpStart ) {
            if ( OpenBracketCount == 0 ) {
                InitBracketIndex = i;
            }
            OpenBracketCount++;
        } else if ( Tokens[ i ].Token == ExpEnd ) {
            OpenBracketCount--;
            if ( OpenBracketCount == 0 ) {
                Root->Children.Add( ConstructRootExpr( Tokens, InitBracketIndex + 1, i - 1 ) );
            }
        } else if ( OpenBracketCount == 0 ) {
            Root->Children.Add( Make_OExprPtr_Data( Tokens[ i ] ) );
        }
    }
    if ( OpenBracketCount != 0 ) {
        cout << endl << "[PARSE_ERROR] Bracket mismatch. Count: " << OpenBracketCount << endl;
    }
    return Root;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens ) {
    return ConstructRootExpr( Tokens, 0, Tokens.Length() - 1 );
}

int CompareTo( const string& LHS, const string& RHS ) {
    if ( Contains( LHS, StrLit[ 0 ] ) || Contains( RHS, StrLit[ 0 ] ) ) {
        return LHS.compare( RHS );
    }

    stringstream ss{};
    ss << LHS;
    float a{};
    ss >> a;

    ss = {};
    ss << RHS;
    float b{};
    ss >> b;

    if ( a < b ) {
        return -1;
    } else if ( a == b ) {
        return 0;
    } else {
        return 1;
    }
}

int Min( int L, int R ) {
    return L <= R ? L : R;
}

const OAtom& TopAtom( const OExprPtr Expr ) {
    if ( Expr->Children.IsNonEmpty() ) {
        return Expr->Children[ 0 ]->Atom;
    }
    return Expr->Atom;
}

const OAtom& LastAtom( const OExprPtr Expr ) {
    if ( Expr->Children.IsNonEmpty() ) {
        return Expr->Children.Last()->Atom;
    }
    return Expr->Atom;
}

void SetFunctionMem( OMachinePtr Machine, const OExprPtr InExpr, const EInExprFuncFormat InExprFuncFormat, const OExprPtr ExprFunc ) {
    // Expr is FUNC VAR1 VAR2 ...
    // Func is NAME VAR1 VAR2 ... BODY
    for ( int ExprIndex = 1; ExprIndex < InExpr->Children.Length(); ExprIndex++ ) {
        // Case: Func has less parameters than we've fed in.
        if ( ExprIndex >= ExprFunc->Children.Length() - 1 ) {
            break;
        }
        OExprPtr NewExpr = Make_OExprPtr( OExprType::Expr );
        NewExpr->Children.Add( ExprFunc->Children[ ExprIndex ] );
        NewExpr->Children.Add( EvalExpr( Machine, InExpr->Children[ ExprIndex ], EEvalIntrinsicMode::Execute ) );
        Machine->Stack.PeekStack().SetOrAdd( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
            if ( ExistingExpr->Children.IsNonEmpty() ) {
                if ( TopAtom( ExistingExpr ).Token.Token == TopAtom( NewExpr ).Token.Token ) {
                    return true;
                }
            }
            return false;
        } );
    }
}

OExprPtr EvalInMemory( const OMachinePtr Machine, const OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode ) {
    for ( int StackFrameIndex = Machine->Stack.Length() - 1; StackFrameIndex >= 0; StackFrameIndex-- ) {
        const OExprList& StackFrame = Machine->Stack[ StackFrameIndex ];
        for ( int i = 0; i < StackFrame.Length(); i++ ) {
            if ( StackFrame[ i ]->Type == OExprType::ExprFunc ) {
                if ( TopAtom( StackFrame[ i ] ).Token.Token == TopAtom( Expr ).Token.Token ) {
                    return EvalNamedFunction( Machine, Expr, StackFrame[ i ], EvalIntrinsicMode );
                }
            } else if ( StackFrame[ i ]->Children.Length() == 1 ) {
                if ( TopAtom( StackFrame[ i ] ).Token.Token == TopAtom( Expr ).Token.Token ) {
                    return EvalExpr( Machine, StackFrame[ i ]->Children[ 0 ], EvalIntrinsicMode );
                }
            } else if ( StackFrame[ i ]->Children.Length() == 2 ) {
                if ( TopAtom( StackFrame[ i ] ).Token.Token == TopAtom( Expr ).Token.Token ) {
                    return EvalExpr( Machine, StackFrame[ i ]->Children[ 1 ], EvalIntrinsicMode );
                }
            }
        }
    }
    return Expr;
}

bool AllData( const OExprPtr Expr ) {
    for ( int i = 0; i < Expr->Children.Length(); i++ ) {
        if ( Expr->Type != OExprType::Data ) {
            return false;
        }
    }
    return true;
}

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, const EEvalIntrinsicMode EvalIntrinsicMode, const EEvalExprReturnMode ReturnMode ) {
    Expr = EvalInMemory( Machine, Expr, EvalIntrinsicMode );

#if PRINT_EVAL
    if ( Expr->Children.IsEmpty() ) {
        std::cout << TopAtom( Expr ).Token << "|";
    }
    for ( int i = 0; i < Expr->Children.Length(); i++ ) {
        std::cout << TopAtom( Expr->Children[ i ] ).Token << "|";
    }
    std::cout << std::endl;
#endif

    // Check if first node is intrinsic.
    if ( EvalIntrinsicMode == EEvalIntrinsicMode::Execute) {
        OIntrinsicPtr Intrinsic = FindIntrinsic( Machine, Expr );
        if ( Intrinsic != Machine->EmptyIntrinsic ) {
            return Intrinsic->Function( Expr );
        }
    }

    for ( int i = 0; i < Expr->Children.Length(); i++ ) {
        OExprPtr ExprOut = EvalExpr( Machine, Expr->Children[ i ], EvalIntrinsicMode );
        Expr->Children[ i ]->Atom = ExprOut->Atom;
        if ( ExprOut->Type == OExprType::Break ) {
            if ( ExprOut->Children.Length() >= 1 ) {
                return ExprOut->Get( 0 );
            } else {
                return ExprOut;
            }
        }
    }

    if ( ReturnMode == EEvalExprReturnMode::LastChild && Expr->Children.IsNonEmpty() ) {
        return Expr->Children.Last();
    }

    return Expr;
}

OExprPtr EvalNamedFunction( OMachinePtr Machine, const OExprPtr Expr, const OExprPtr Function, const EEvalIntrinsicMode EvalIntrinsicMode ) {
    // Params are child [1, (N-2)], body is N-1
    Machine->Stack.PushStack();
    SetFunctionMem( Machine, Expr, EInExprFuncFormat::FirstTokenName, Function );
    OExprPtr Out = EvalExpr( Machine, Function->Children.Last(), EvalIntrinsicMode );
    // We want to remove child nodes because they are structures only of the Function
    Machine->Stack.PopStack();
    return Make_OExprPtr_Data( Expr->Atom, Out->Atom.Token.Token);
}

OExprPtr EvalExpr( OMachinePtr Machine, const OExprPtr Expr, const EEvalIntrinsicMode EvalIntrinsicMode ) {
    return EvalExpr( Machine, Expr, EvalIntrinsicMode, EEvalExprReturnMode::LastChild );
}

void ResetMachine( OMachinePtr Machine ) {
    Machine->EmptyIntrinsic = {};
    Machine->Intrinsics.Clear();
    Machine->Stack.Clear();
    Machine->ShouldExit = false;
    BuildIntrinsics( Machine );
    Machine->Stack.PushStack();
}

OExprPtr Execute( OMachinePtr Machine, OExprPtr Program ) {
    OExprPtr Ret = EvalExpr( Machine, Program, EEvalIntrinsicMode::Execute );
    return Ret;
}

void InterpreterLoop( OMachinePtr Machine ) {
    Machine->Stack.PushStack();
    while ( !Machine->ShouldExit ) {
        string Input;
        std::getline( std::cin, Input );
        const TokenList Tokens = Tokenize( Input );
        const OExprPtr Program = ConstructRootExpr( Tokens );
        OExprPtr Out = Execute( Machine, Program );
        cout << Out->Atom.Token.Token<< endl;
    }
    Machine->Stack.PopStack();
}
