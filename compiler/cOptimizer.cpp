#include <iostream>
#include "cOptimizer.h"
#include "lex.h"

//************************************************
cOptimizer::cOptimizer(const std::unordered_map<int, bool>& flags, int oLevel) : cVisitor()
{
    m_flags = flags;

    switch(oLevel)
    {
        case 3:
        case 2:
        case 1:
            EnableFlag(oFlags::reduce_exprs);
            EnableFlag(oFlags::reduce_const_vars);
            EnableFlag(oFlags::reduce_branches);
        default: break;
    }
}

cOptimizer::~cOptimizer()
{
}

//************************************************
void cOptimizer::VisitAllNodes(cAstNode *node)
{
    node->Visit(this);
}

//************************************************
void cOptimizer::ClearChildren()
{
    if (m_ParentStack.empty()) return;
    cAstNode *parent = m_ParentStack.top();
    for (int child = 0; child < parent->NumChildren(); ++child)
        parent->SetChild(child, nullptr);
}

void cOptimizer::ReplaceChild(cAstNode *existingNode, cAstNode *newNode)
{
    if (m_ParentStack.empty()) return;
    cAstNode *parent = m_ParentStack.top();

    for (int child = 0; child < parent->NumChildren(); ++child)
    {
        if (parent->GetChild(child) == existingNode)
        {
            parent->SetChild(child, newNode);
            break;
        }
    }
}

//************************************************
static bool CanReduce(cExpr *node)
{
    if (node->IsArray()) return false;
    if (node->IsPointer()) return false;
    if (!node->IsConst()) return false;

    return true;
}

static bool CanReduce(cDecl *node)
{
    return node->IsConst() && node->IsInt();
}

//************************************************
void cOptimizer::Visit(cBinaryExpr *node)
{
    if (FlagIsEnabled(oFlags::reduce_exprs) && CanReduce(node))
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cCastExpr *node)
{
    cExpr* expr = node->GetExpr();
    if (FlagIsEnabled(oFlags::reduce_exprs) && CanReduce(expr))
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cPlainVarRef *node)
{
    if (FlagIsEnabled(oFlags::reduce_const_vars) && CanReduce(node))
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cSizeofExpr *node)
{
    if (FlagIsEnabled(oFlags::reduce_exprs) && node->IsConst())
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cUnaryExpr *node)
{
    if (FlagIsEnabled(oFlags::reduce_exprs) && CanReduce(node))
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cIfStmt *node)
{
    m_ParentStack.push(node); 
    VisitAllChildren(node); 
    m_ParentStack.pop();

    cExpr *cond = node->GetCond();
    if (FlagIsEnabled(oFlags::reduce_branches) && CanReduce(cond))
    {
        if (cond->ConstValue()) ReplaceChild(node, node->GetIfStmt());
        else ReplaceChild(node, node->GetElseStmt());
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cShortCircuitExpr *node)
{
    if (FlagIsEnabled(oFlags::reduce_exprs) && CanReduce(node))
    {
        cIntExpr *intExpr = new cIntExpr(node->ConstValue());
        ReplaceChild(node, intExpr);
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cVarDecl *node)
{
    if (FlagIsEnabled(oFlags::reduce_const_vars) && CanReduce(node))
    {
        ReplaceChild(node, nullptr);
        return;
    }
    else
    {
        m_ParentStack.push(node); 
        VisitAllChildren(node); 
        m_ParentStack.pop();
    }
}

void cOptimizer::Visit(cAddressExpr *node)      { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cArrayRef *node)         { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cArrayType *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cAsmNode *node)          { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cAssignExpr *node)       { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cAstNode *node)          { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cBaseDeclNode *node)     { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cBreakStmt *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cContinueStmt *node)     { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cDecl *node)             { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cDeclsList *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cDoWhileStmt *node)      { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cExpr *node)             { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cExprStmt *node)         { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cForStmt *node)          { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cFuncCall *node)         { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cFuncDecl *node)         { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cIntExpr *node)          { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cNopStmt *node)          { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cParams *node)           { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cPointerDeref *node)     { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cPointerType *node)      { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cPostfixExpr *node)      { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cPragma *node)           { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cPrefixExpr *node)       { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cReturnStmt *node)       { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStmt *node)             { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStmtsList *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStringLit *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStructDeref *node)      { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStructRef *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cStructType *node)       { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cSymbol *node)           { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cTypeDecl *node)         { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cVarRef *node)           { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }
void cOptimizer::Visit(cWhileStmt *node)        { m_ParentStack.push(node); VisitAllChildren(node); m_ParentStack.pop(); }