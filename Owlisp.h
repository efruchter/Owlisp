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
typedef OArray<OExprList> StackFrames;
typedef OArray<OIntrinsicPtr> OIntrinsics;
typedef function<OExprPtr( const OExprPtr )> IntrinsicFunction;

const string TOKEN_DEFUNC = "defunc";
const string TOKEN_FALSE = "0";
const string TOKEN_TRUE = "1";

enum class EEvalIntrinsicMode {
    NoExecute,
    Execute
};

enum class EEvalExprReturnMode {
    TopExpr,
    LastChild
};

enum class OExprType {
    // A native implemented Function. Will have a function pointer.
    NativeFunction,
    // An evaluatable expression root
    Expr,
    // Data, cannot be expanded.
    Data,
    // A function, needs own stack frame w. parameters loaded into it.
    ExprFunc,
    //Break the enclosing loop
    Break
};

struct OIntrinsic {
    OExprType Type;
    string Token;
    IntrinsicFunction Function;
};

enum class OAtomDataPrimitiveType : char {
    String,
    Int,
    Float,
    Long,
    Double
};

union OAtomData {
    int Int;
    float Float;
    long Long;
    double Double;
};

struct OAtom {
    OAtomData PrimitiveData{};
    OAtomDataPrimitiveType PrimitiveType{};
    string Token{};
};

struct OExpr {
    OExprType Type{};
    OAtom Atom{};
    OExprList Children{};

    OExprPtr& Get( const int Index ) {
        return Children[ Index ];
    }
};

struct OMachine {
    OIntrinsicPtr EmptyIntrinsic;
    OIntrinsics Intrinsics;
    StackFrames Stack;
    bool ShouldExit;
};

OExprPtr EvalNamedFunction( OMachinePtr Machine, const OExprPtr Expr, const OExprPtr Function, const EEvalIntrinsicMode EvalIntrinsicMode );

OExprPtr Make_OExprPtr_Empty();
OExprPtr Make_OExprPtr( const OExprType Type );
OExprPtr Make_OExprPtr_Data( const string& Token );
OIntrinsicPtr Make_OIntriniscPtr( const OExprType Type );
OMachinePtr Make_OMachinePtr();

OExprPtr ToOExpr_SingleNoEval( const TokenList& Tokens );

OExprPtr ConstructRootExpr( const TokenList& Tokens, int StartIndex, int EndIndex );
OExprPtr ConstructRootExpr( const TokenList& Tokens );

enum class EInExprFuncFormat {
    FirstTokenName
};

void SetFunctionMem( OMachinePtr Machine, const OExprPtr InExpr, const EInExprFuncFormat InExprFuncFormat, const OExprPtr ExprFunc );
OExprPtr EvalInMemory( const OMachinePtr Machine, const OExprPtr Expr, EEvalIntrinsicMode EvalIntrinsicMode );

OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, const EEvalIntrinsicMode EvalIntrinsicMode );
OExprPtr EvalExpr( OMachinePtr Machine, OExprPtr Expr, const EEvalIntrinsicMode EvalIntrinsicMode, const EEvalExprReturnMode ReturnMode );

const OAtom& TopAtom( const OExprPtr Expr );
const OAtom& LastAtom( const OExprPtr Expr );

void ResetMachine( OMachinePtr Machine );
void BuildIntrinsics( OMachinePtr Machine );

OExprPtr Execute( OMachinePtr Machine, OExprPtr Program );

void InterpreterLoop( OMachinePtr Machine );

