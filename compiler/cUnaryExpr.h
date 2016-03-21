#pragma once

#include <string>

#include "cExpr.h"
#include "cSymbol.h"

class cUnaryExpr : public cExpr
{
  public:
    cUnaryExpr(int op, cExpr *expr) : cExpr() 
    {
        AddChild(expr);
        mOp = op;
    }

    virtual bool IsConst() { return GetExpr()->IsConst(); }
    virtual int  ConstValue()
    {
        switch (mOp)
        {
            case '-':
                return -GetExpr()->ConstValue();
            case '~':
                return ~GetExpr()->ConstValue();
            case '!':
                return !GetExpr()->ConstValue();
            default:
                fatal_error("Unrecognized unary operator");
                break;
        }

        return 0;
    }

    virtual cTypeDecl *GetType() { return GetExpr()->GetType(); }

    std::string OpToString()
    {
        switch (mOp)
        {
            case '-':
                return "-";
            case '~':
                return "~";
            case '!':
                return "!";
            default:
                fatal_error("Unrecognized unary operator");
                break;
        }

        return "unrecognized unary op ";
    }

    cExpr* GetExpr()    { return (cExpr*)GetChild(0); }
    int    GetOp()      { return mOp; }

    virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }
  protected:
    int   mOp;
};

