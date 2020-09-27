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

typedef OExpr* OExprPtr;
typedef OMachine* OMachinePtr;
typedef OIntrinsic* OIntrinsicPtr;

typedef OArray<OExprPtr> OExprList;
typedef OArray<OIntrinsicPtr> OIntrinsics;
typedef function<OExprPtr( const OExprPtr )> IntrinsicFunction;

enum class EEvalIntrinsicMode {
	NoExecute,
	Execute
};

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
	OExprList Memory;
	bool ShouldExit;
};

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode );

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
	{ // exit
		const string Token_Exit = "exit";
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
		Intrinsic->Token = Token_Exit;
		Intrinsic->Function = [Token_Exit, Machine]( const OExprPtr Expr ) {
			Machine->ShouldExit = true;
			return Make_OExprPtr_Empty();
		};
		Machine->Intrinsics.Add( Intrinsic );
	}
	{ // print
		const string Token_Print = "print";
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
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
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
		Intrinsic->Token = Token_Print;
		Intrinsic->Function = [Token_Print, Machine]( const OExprPtr Expr ) {
			assert( Expr->Children.Length() > 0 );
			assert( Expr->Children[ 0 ]->Atom.Token == Token_Print );
			for ( int i = 1; i < Expr->Children.Length(); i++ ) {
				cout << TrimEnclosingQuotes( EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token ) << endl;
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
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
		Intrinsic->Token = Token_Addition;
		Intrinsic->Function = [Token_Addition, Machine]( const OExprPtr Expr ) {
			assert( Expr->Children.Length() > 0 );
			assert( Expr->Children[ 0 ]->Atom.Token == Token_Addition );
			int sum = 0;
			for ( int i = 1; i < Expr->Children.Length(); i++ ) {
				stringstream Stream;
				Stream << EvalExpr( Machine, Expr->Children[ i ], EEvalIntrinsicMode::Execute )->Atom.Token;
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
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
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

	{ // / Floating Point Division
		const string Token_Div = "/";
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
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
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
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
		const string Token_Set = "set";
		OIntrinsicPtr Intrinsic = Make_OIntriniscPtr( OExprType::Intrinsic );
		Intrinsic->Token = Token_Set;
		Intrinsic->Function = [Token_Set, Machine]( const OExprPtr Expr ) {
			assert( Expr->Children.Length() == 3 );
			assert( Expr->Children[ 0 ]->Atom.Token == Token_Set );
			assert( Expr->Children[ 1 ]->Atom.Token != "" );

			OExprPtr NewExpr = Make_OExprPtr( OExprType::Expr );
			NewExpr->Children.Add( Expr->Children[ 1 ] );
			NewExpr->Children.Add( EvalExpr( Machine, Expr->Children[ 2 ], EEvalIntrinsicMode::NoExecute ) );

			Machine->Memory.SwapOrSet( NewExpr, [&]( const OExprPtr& ExistingExpr ) {
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

	// Needs more expension

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

	assert( OpenBracketCount == 0 );

	return Root;
}

OExprPtr ConstructRootExpr( const TokenList& Tokens ) {
	return ConstructRootExpr( Tokens, 0, Tokens.Length() - 1 );
}

OExprPtr EvalInMemory( const OMachinePtr Machine, const OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode ) {
	for ( int i = 0; i < Machine->Memory.Length(); i++ ) {
		if ( Machine->Memory[ i ]->Children.Length() == 1 ) {
			if ( Machine->Memory[ i ]->Children[ 0 ]->Atom.Token == Expr->Atom.Token ) {
				return EvalExpr( Machine, Machine->Memory[ i ]->Children[ 0 ], EvalIntrinsicMode );
			}
		} else if ( Machine->Memory[ i ]->Children.Length() == 2 ) {
			if ( Machine->Memory[ i ]->Children[ 0 ]->Atom.Token == Expr->Atom.Token ) {
				return EvalExpr( Machine, Machine->Memory[ i ]->Children[ 1 ], EvalIntrinsicMode );
			}
		}
	}
	return Expr;
}

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode ) {
	if ( Expr->Type == OExprType::Data ) {
		return EvalInMemory( Machine, Expr, EvalIntrinsicMode );
	}

	if ( Expr->Type == OExprType::Expr ) {
		for ( int i = 1; i < Expr->Children.Length(); i++ ) {
			if ( Expr->Children[ i ]->Type == OExprType::Expr ) {
				Expr->Children[ i ]->Atom = EvalExpr( Machine, Expr->Children[ i ], EvalIntrinsicMode )->Atom;
			}
		}

		if ( EvalIntrinsicMode == EEvalIntrinsicMode::Execute ) {
			OIntrinsicPtr Intrinsic = FindIntrinsic( Machine, Expr );
			if ( Intrinsic != Machine->EmptyIntrinsic ) {
				return Intrinsic->Function( Expr );
			}
		} else {
			return Expr;
		}
	}

	return Make_OExprPtr_Empty();
}

void ResetMachine( OMachinePtr Machine ) {
	*Machine = {};
	BuildIntrinsics( Machine );
}

OExprPtr Execute( OMachinePtr Machine, OExprPtr Program ) {
	if ( Program->Type == OExprType::ExprList ) {
		if ( Program->Children.Length() == 1 ) {
			return EvalExpr( Machine, Program->Children[0], EEvalIntrinsicMode::Execute );
		}
		for ( int i = 0; i < Program->Children.Length(); i++ ) {
			EvalExpr( Machine, Program->Children[ i ], EEvalIntrinsicMode::Execute );
		}
	} else {
		return EvalExpr( Machine, Program, EEvalIntrinsicMode::Execute );
	}
	return Make_OExprPtr_Empty();
}

void InterpreterLoop( OMachinePtr Machine ) {
	while ( !Machine->ShouldExit ) {
		string Input;
		std::getline ( std::cin, Input );
		const TokenList Tokens = Tokenize( Input );
		const OExprPtr Program = ConstructRootExpr( Tokens );
		OExprPtr Out = Execute( Machine, Program );
		cout << Out->Atom.Token << endl;
	}
}

int main(int argc, char *argv[]) {
	OMachinePtr Machine = Make_OMachinePtr();
	ResetMachine( Machine );

	if ( argc > 1 ) {
		if ( string{ argv[1] } == "-i" ) {
			InterpreterLoop( Machine );
			return 0;
		}
	}

	const string ToParse1 =
		"  (println `ABC` `EFG`)  ";

	const string ToParse2 =
		" ( set X 3 ) "
		" ( set Y 4 ) "
		" (println (+ (* X Y ) ( Y ) ) ) ";

	const string ToParse3 =
		" (set CurrentYear 2020)"
		" (set FoundingYear 1776)"
		" (set AvgGenerationLength 30) "
		" (print `Generations since founding: ` (/ (- CurrentYear FoundingYear) AvgGenerationLength) `\n`) "
		;

	const TokenList Tokens = Tokenize( ToParse3 );
	const OExprPtr Program = ConstructRootExpr( Tokens );
	Execute( Machine, Program );

	//cout << "Tokens:" << endl;
	//for ( int i = 0; i < Tokens.Length(); i++ ) {
	//    cout << Tokens[ i ] << endl;
	//}
	//cout << endl << endl << "Output:" << endl << endl;
	
	return 0;
}

