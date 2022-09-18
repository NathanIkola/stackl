#pragma once

#include "cStmt.h"
#include "cExpr.h"

class cSwitchStmt : public cStmt
{
    public:
        cSwitchStmt(cExpr *condition, cStmt *body) : cStmt()
        {
            AddChild(condition);
            AddChild(body);
        }

        cExpr* GetExpr() { return (cExpr*)GetChild(0); }
        cStmt* GetBody() { return (cStmt*)GetChild(1); }

        virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }
    private:
};