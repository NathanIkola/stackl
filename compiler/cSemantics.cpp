#include <iostream>
#include "cSemantics.h"
#include "lex.h"

#include <stack>
#include <unordered_map>

using std::string;
using FunctionLabelScope = std::unordered_map<string, int>;

//***********************************
cSemantics::cSemantics() : cVisitor()
{
}

cSemantics::~cSemantics()
{
}

void cSemantics::VisitAllNodes(cAstNode *node) 
{ 
    node->Visit(this); 
}

void cSemantics::Visit(cBreakStmt *node)
{
    if (m_jumpContextLevel == 0)
    {
        semantic_error("Break statement not within a loop or switch", node->LineNumber());
    }
    VisitAllChildren(node);
}

void cSemantics::Visit(cCaseStmt *node)
{
    if (m_jumpContextLevel <= 0)
    {
        semantic_error("Case statement encountered outside of the body of a switch statement", node->LineNumber());
    }
    else 
    {
        if (!node->IsDefault())
        {
            if (!cTypeDecl::IsCompatibleWith(m_switchType, node->GetExpr()->GetType()))
            {
                semantic_error("Case statement type is incompatible with switch condition", node->LineNumber());
            }

            if (m_switchCases.find(node->GetExpr()->ConstValue()) != m_switchCases.end())
            {
                semantic_error("Duplicate cases defined", node->LineNumber());
            }
            else { m_switchCases.insert({node->GetExpr()->ConstValue(), true}); }
        }
        else  
        {
            if (m_switchHasDefault)
            {
                semantic_error("Default case already defined", node->LineNumber()); 
            }
            m_switchHasDefault = true;
        }
    }
    VisitAllChildren(node);
}

void cSemantics::Visit(cContinueStmt *node)
{
    if (m_jumpContextLevel == 0)
    {
        semantic_error("Continue statement not within a loop", node->LineNumber());
    }
    VisitAllChildren(node);
}

void cSemantics::Visit(cDoWhileStmt *node)
{
    m_jumpContextLevel += 1;
    VisitAllChildren(node);
    m_jumpContextLevel -= 1;
}

void cSemantics::Visit(cForStmt *node)
{
    m_jumpContextLevel += 1;
    VisitAllChildren(node);
    m_jumpContextLevel -= 1;
}

void cSemantics::Visit(cFuncDecl *node)
{
    m_funcReturnType = node->ReturnType();

    IncreaseFunctionScope();
    VisitAllChildren(node);
    DecreaseFunctionScope();

    // Force a return statement
    if (node->IsDefinition())
    {
        if (!node->ReturnType()->IsVoid())
        {
            node->GetStmts()->AddChild(new cReturnStmt(new cIntExpr(0)));
        }
        else
        {
            node->GetStmts()->AddChild(new cReturnStmt(NULL));
        }
    }
}

void cSemantics::Visit(cGotoStmt *node)
{
    if (!InFunctionScope())
    {
        semantic_error("Goto encountered outside of function body", node->LineNumber());
    }
    else if (!LabelExists(node->GetLabel()))
    {
        SetLabelLine(node->GetLabel(), node->LineNumber());
    }
}

void cSemantics::Visit(cLabeledStmt *node)
{
    if (!InFunctionScope())
    {
        semantic_error("Label \"" + node->GetLabel() + "\" declared outside of function body", node->LineNumber());
    }
    else if (LabelExists(node->GetLabel()) && GetLabelLine(node->GetLabel()) == -1)
    {
        semantic_error("Duplicate label \"" + node->GetLabel() + "\"", node->LineNumber());
    }
    else
    {
        SetLabelLine(node->GetLabel(), -1);
    }
    VisitAllChildren(node);
}

void cSemantics::Visit(cReturnStmt *node)
{
    if (m_funcReturnType->IsVoid() && node->GetExpr() == nullptr) return;
    if (m_funcReturnType->IsVoid() && node->GetExpr() != nullptr) 
    {
        semantic_error("Return statement type is incompatible with function declaration",
            node->LineNumber());
    }
    else if (!m_funcReturnType->IsVoid() && node->GetExpr() == nullptr)
    {
        semantic_error("Return statement type is incompatible with function declaration",
            node->LineNumber());
    }
    else if (!cTypeDecl::IsCompatibleWith(m_funcReturnType, 
                                     node->GetExpr()->GetType()))
    {
        semantic_error("Return statement type is incompatible with function declaration",
            node->LineNumber());
    }
}

void cSemantics::Visit(cSwitchStmt *node)
{
    m_jumpContextLevel += 1;
    bool oldHasDefault = m_switchHasDefault;
    std::unordered_map<int, bool> oldCases = m_switchCases;
    m_switchCases = {};
    m_switchHasDefault = false;
    cTypeDecl* outerSwitchType = m_switchType;
    m_switchType = node->GetExpr()->GetType();

    VisitAllChildren(node);

    m_switchHasDefault = oldHasDefault;
    m_switchCases = oldCases;
    m_switchType = outerSwitchType;
    m_jumpContextLevel -= 1;
}

void cSemantics::Visit(cWhileStmt *node)
{
    m_jumpContextLevel += 1;
    VisitAllChildren(node);
    m_jumpContextLevel -= 1;
}

//***********************************
void cSemantics::IncreaseFunctionScope() { m_funcLabelStack.emplace(FunctionLabelScope{}); }
void cSemantics::DecreaseFunctionScope()
{
    for (auto label : m_funcLabelStack.top())
    {
        if (label.second != -1)
            semantic_error("Reference to undefined label \"" + label.first + "\"", label.second);
    }
    m_funcLabelStack.pop();
}

bool cSemantics::InFunctionScope()                          { return !m_funcLabelStack.empty(); }
bool cSemantics::LabelExists(string label)                  { return m_funcLabelStack.top().find(label) != m_funcLabelStack.top().end(); }
void cSemantics::SetLabelLine(string label, int lineNumber) { m_funcLabelStack.top()[label] = lineNumber; }
int cSemantics::GetLabelLine(string label)                  { return InFunctionScope() ? m_funcLabelStack.top().find(label)->second : -2; }