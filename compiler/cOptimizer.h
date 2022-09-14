#pragma once

#include <stack>
#include <unordered_map>
#include "cVisitor.h"

class cOptimizer : public cVisitor
{
    public:
        cOptimizer(const std::unordered_map<int,bool>& flags, int oLevel = 0);
        ~cOptimizer();

        virtual void VisitAllNodes(cAstNode *node);

        virtual void Visit(cAddressExpr *node);
        virtual void Visit(cArrayRef *node);
        virtual void Visit(cArrayType *node);
        virtual void Visit(cAsmNode *node);
        virtual void Visit(cAssignExpr *node);
        virtual void Visit(cAstNode *node);
        virtual void Visit(cBaseDeclNode *node);
        virtual void Visit(cBinaryExpr *node);
        virtual void Visit(cBreakStmt *node);
        virtual void Visit(cCastExpr *node);
        virtual void Visit(cContinueStmt *node);
        virtual void Visit(cDecl *node);
        virtual void Visit(cDeclsList *node);
        virtual void Visit(cDoWhileStmt *node);
        virtual void Visit(cExpr *node);
        virtual void Visit(cExprStmt *node);
        virtual void Visit(cForStmt *node);
        virtual void Visit(cFuncCall *node);
        virtual void Visit(cFuncDecl *node);
        virtual void Visit(cIfStmt *node);
        virtual void Visit(cIntExpr *node);
        virtual void Visit(cNopStmt *node);
        virtual void Visit(cParams *node);
        virtual void Visit(cPlainVarRef *node);
        virtual void Visit(cPointerDeref *node);
        virtual void Visit(cPointerType *node);
        virtual void Visit(cPostfixExpr *node);
        virtual void Visit(cPragma *node);
        virtual void Visit(cPrefixExpr *node);
        virtual void Visit(cReturnStmt *node);
        virtual void Visit(cShortCircuitExpr *node);
        virtual void Visit(cSizeofExpr *node);
        virtual void Visit(cStmt *node);
        virtual void Visit(cStmtsList *node);
        virtual void Visit(cStringLit *node);
        virtual void Visit(cStructDeref *node);
        virtual void Visit(cStructRef *node);
        virtual void Visit(cStructType *node);
        virtual void Visit(cSymbol *node);
        virtual void Visit(cTypeDecl *node);
        virtual void Visit(cUnaryExpr *node);
        virtual void Visit(cVarDecl *node);
        virtual void Visit(cVarRef *node);
        virtual void Visit(cWhileStmt *node);

        enum oFlags
        {
            reduce_exprs = 1,
            reduce_const_vars,
            reduce_branches,
        };
    protected:
        std::stack<cAstNode*> m_ParentStack;
        std::unordered_map<int,bool> m_flags;
        inline bool FlagIsEnabled(int flag) { return m_flags.find(flag) != m_flags.end(); }
        inline void EnableFlag(int flag) { m_flags.emplace(flag, true); }
        void ClearChildren();
        void ReplaceChild(cAstNode *existingNode, cAstNode *newNode);
};