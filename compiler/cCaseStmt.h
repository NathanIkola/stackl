#pragma once

#include "cStmt.h"
#include "cExpr.h"

class cCaseStmt : public cStmt
{
    public:
        cCaseStmt(cExpr *expr, cStmt* stmt) : cStmt()
        {
            AddChild(expr);
            AddChild(stmt);
            mIsDefault = false;
        }

        cCaseStmt(cStmt *stmt) : cStmt()
        {
            AddChild(nullptr);
            AddChild(stmt);
            mIsDefault = true;
        }

        inline bool IsDefault() { return mIsDefault; }
        inline cExpr* GetExpr() { return (cExpr*)GetChild(0); }
        inline cStmt* GetStmt() { return (cStmt*)GetChild(1); }

        virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }
    private:
        bool mIsDefault;
};