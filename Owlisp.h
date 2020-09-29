#pragma once

#include "Containers.h"
#include "Tokenizer.h"
#include "IO.h"


struct OExpr;
struct OAtom;
struct OMachine;
struct OIntrinsic;

typedef unsigned int uint;

#if MANAGE_EXPR_MEM
typedef shared_ptr<OExpr> OExprPtr;
typedef shared_ptr<OMachine> OMachinePtr;
typedef shared_ptr<OIntrinsic> OIntrinsicPtr;
#else
typedef OExpr* OExprPtr;
typedef OMachine* OMachinePtr;
typedef OIntrinsic* OIntrinsicPtr;
#endif

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

OExprPtr Make_OExprPtr_Empty();
OExprPtr Make_OExprPtr( const OExprType Type );
OExprPtr Make_OExprPtr_Data( const string& Token );
OIntrinsicPtr Make_OIntriniscPtr( const OExprType Type );
OMachinePtr Make_OMachinePtr();

OExprPtr ToOExpr_SingleNoEval( const TokenList& Tokens );

const OIntrinsicPtr FindFunc( const OMachinePtr Machine, const OExprPtr Expr );

OExprPtr ConstructRootExpr( const TokenList& Tokens, int StartIndex, int EndIndex );
OExprPtr ConstructRootExpr( const TokenList& Tokens );

OExprPtr EvalInMemory( const OMachinePtr Machine, const OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode );
OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode );

void ResetMachine( OMachinePtr Machine );
void BuildIntrinsics( OMachinePtr Machine );

OExprPtr Execute( OMachinePtr Machine, OExprPtr Program );

void InterpreterLoop( OMachinePtr Machine );

