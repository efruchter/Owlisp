#define MANAGE_EXPR_MEM 0
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

OExprPtr Make_OExprPtr_Data( const string& Token ) {
    OExprPtr AtomExpr = Make_OExprPtr( OExprType::Data );
    AtomExpr->Atom.Token = Token;
    return AtomExpr;
}

OExprPtr Make_OExprPtr_DataExprCap( bool StartCap ) {
    return Make_OExprPtr_Data( StartCap ? ExpStart : ExpEnd );
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
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                cout << TrimEnclosingQuotes( EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token );
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
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                OExprPtr Result = EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute );
                cout << TrimEnclosingQuotes( Result->Atom.Token ) << endl;
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
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Addition );
            int sum = 0;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                sum += ParseTokenToPrimitive<int>( EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token );
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
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Sub;
        Intrinsic->Function = [Token_Sub, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Sub );
            int sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token;
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
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Mul;
        Intrinsic->Function = [Token_Mul, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Mul );
            float sum = 0;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token;
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
    { // sqrt
        const string Token_Sqrt = "sqrt";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Sqrt;
        Intrinsic->Function = [Token_Sqrt, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() == 2 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Sqrt );
            stringstream Stream;
            Stream << EvalExpr( Machine, Expr->Children[ 1 ], EEvalIntrinsicMode::Execute )->Atom.Token;
            float a = 0;
            Stream >> a;
            OExprPtr Result = Make_OExprPtr( OExprType::Data );
            stringstream SS{};
            SS << std::sqrtf( a );
            Result->Atom.Token = SS.str();
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
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Div );
            float sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token;
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
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_IDiv;
        Intrinsic->Function = [Token_IDiv, Machine]( const OExprPtr Expr ) {
            assert( Expr->Children.Length() > 0 );
            assert( Expr->Children[ 0 ]->Atom.Token == Token_IDiv );
            int sum = 1;
            bool set = false;
            for ( int i = 1; i < Expr->Children.Length(); i++ ) {
                stringstream Stream;
                Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token;
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
    { // Set
        const string Token_Set = "=";
        OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::NativeFunction );
        Intrinsic->Token = Token_Set;
        Intrinsic->Function = [Token_Set, Machine]( const OExprPtr Expr ) {
            const int KeyIndex = 1;
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Set );
            assert( Expr->Children.Length() == KeyIndex + 2 );
            OExprPtr NewExpr = Make_OExprPtr( OExprType::Expr );
            NewExpr->Children.Add( Expr->Children[ KeyIndex ] );
            NewExpr->Children.Add( EvalExpr( Machine, Expr->Children[ KeyIndex + 1 ], EEvalIntrinsicMode::NoExecute ) );
            Machine->Stack.PeekStack().SetOrAdd( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
                if ( ExistingExpr->Children.Length() == 2 ) {
                    if ( ExistingExpr->Children[ 0 ]->Atom.Token == NewExpr->Children[ 0 ]->Atom.Token ) {
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
            assert( Expr->Children[ 0 ]->Atom.Token == Token_Defunc );
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
                    if ( ExistingExpr->Children[ 0 ]->Atom.Token == NewExpr->Children[ 0 ]->Atom.Token ) {
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
            assert( Expr->Children.Length() == 4 ); //
            auto Res = EvalExpr( Machine, Expr->Get( 1 ), EEvalIntrinsicMode::Execute );
            if ( Res->Atom.Token == "0" ) {
                return EvalExpr( Machine, Expr->Get( 3 ), EEvalIntrinsicMode::Execute );
            }
            return EvalExpr( Machine, Expr->Get( 2 ), EEvalIntrinsicMode::Execute );
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
            return Make_OExprPtr_Data( TopAtom( LHS ).Token == TopAtom( RHS ).Token ? TOKEN_TRUE : TOKEN_FALSE );
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
            return Make_OExprPtr_Data( TopAtom( LHS ).Token < TopAtom( RHS ).Token ? TOKEN_TRUE : TOKEN_FALSE );
        };
        Machine->Intrinsics.Add( Intrinsic );
    }
}

const OIntrinsicPtr FindIntrinsic( const OMachinePtr Machine, const OExprPtr Expr ) {
    for ( int i = 0; i < Machine->Intrinsics.Length(); i++ ) {
        if ( Machine->Intrinsics[ i ]->Token == TopAtom( Expr ).Token ) {
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
            if ( !InLiteral && Tokens[ i ] == StrLit ) {
                InLiteral = true;
            } else if ( InLiteral && Tokens[ i ] == StrLit ) {
                InLiteral = false;
            } else if ( !InLiteral && ( Tokens[ i ] == ExpStart || Tokens[ i ] == ExpEnd ) ) {
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
    assert( OpenBracketCount == 0 );
    return Root;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens ) {
    return ConstructRootExpr( Tokens, 0, Tokens.Length() - 1 );
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

void SetFunctionMem( OMachinePtr Machine, const OExprPtr Expr, const OExprPtr Function ) {
    // Expr is FUNC VAR1 VAR2 ...
    // Func is NAME VAR1 VAR2 ... BODY
    for ( int ExprIndex = 1; ExprIndex < Expr->Children.Length(); ExprIndex++ ) {
        // Case: Func has less parameters than we've fed in.
        if ( ExprIndex >= Function->Children.Length() - 1 ) {
            break;
        }

        OExprPtr NewExpr = Make_OExprPtr( OExprType::Expr );
        NewExpr->Children.Add( Function->Children[ ExprIndex ] );
        NewExpr->Children.Add( EvalExpr( Machine, Expr->Children[ ExprIndex ], EEvalIntrinsicMode::NoExecute ) );
        Machine->Stack.PeekStack().SetOrAdd( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
            if ( ExistingExpr->Children.IsNonEmpty() ) {
                if ( TopAtom( ExistingExpr ).Token == TopAtom( NewExpr ).Token ) {
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
                if ( TopAtom( StackFrame[ i ] ).Token == TopAtom( Expr ).Token ) {
                    // Params are child [1, (N-2)], body is N-1
                    OExprPtr FunctionExpr = StackFrame[ i ];
                    Machine->Stack.PushStack();
                    SetFunctionMem( Machine, Expr, FunctionExpr );
                    OExprPtr Out = EvalExpr( Machine, FunctionExpr->Children.Last(), EvalIntrinsicMode );
                    // We want to remove child nodes because they are structures only of the Function
                    Out->Children.Clear();
                    Machine->Stack.PopStack();
                    return Out;
                }
            } else if ( StackFrame[ i ]->Children.Length() == 1 ) {
                if ( TopAtom( StackFrame[ i ] ).Token == TopAtom( Expr ).Token ) {
                    return EvalExpr( Machine, StackFrame[ i ]->Children[ 0 ], EvalIntrinsicMode );
                }
            } else if ( StackFrame[ i ]->Children.Length() == 2 ) {
                if ( TopAtom( StackFrame[ i ] ).Token == TopAtom( Expr ).Token ) {
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

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode ) {
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
    if ( EvalIntrinsicMode == EEvalIntrinsicMode::Execute && Expr->Children.IsNonEmpty() ) {
        OIntrinsicPtr Intrinsic = FindIntrinsic( Machine, Expr );
        if ( Intrinsic != Machine->EmptyIntrinsic ) {
            return Intrinsic->Function( Expr );
        }
    }

    for ( int i = 0; i < Expr->Children.Length(); i++ ) {
        Expr->Children[ i ]->Atom = EvalExpr( Machine, Expr->Children[ i ], EvalIntrinsicMode )->Atom;
    }

    if ( Expr->Children.IsNonEmpty() ) {
        return Expr->Children.Last();
    }

    return Expr;
}

void ResetMachine( OMachinePtr Machine ) {
    Machine->EmptyIntrinsic = {};
    Machine->Intrinsics.Clear();
    Machine->Stack.Clear();
    Machine->ShouldExit = false;
    BuildIntrinsics( Machine );
}

OExprPtr Execute( OMachinePtr Machine, OExprPtr Program ) {
    Machine->Stack.PushStack();
    OExprPtr Ret = EvalExpr( Machine, Program, EEvalIntrinsicMode::Execute );
    Machine->Stack.PopStack();
    return Ret;
}

void InterpreterLoop( OMachinePtr Machine ) {
    while ( !Machine->ShouldExit ) {
        string Input;
        std::getline( std::cin, Input );
        const TokenList Tokens = Tokenize( Input );
        const OExprPtr Program = ConstructRootExpr( Tokens );
        OExprPtr Out = Execute( Machine, Program );
        cout << Out->Atom.Token << endl;
    }
}
