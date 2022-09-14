#pragma once

#include <string>

#include "cDecl.h"
#include "cVarDecl.h"
#include "cExpr.h"
#include "cSymbol.h"
#include "cVarRef.h"

class cPlainVarRef : public cVarRef
{
  public:
    cPlainVarRef(cSymbol *id) : cVarRef() 
    {
        if (symbolTableRoot->Lookup(id->Name()) == nullptr)
        {
            ThrowSemanticError(id->Name() + " is not defined");
            return;
        }

        // Was cast to cDecl
        if (dynamic_cast<cDecl *>( id->GetDecl()) == nullptr)
        {
            ThrowSemanticError(id->Name() + " is not declared");
            return;
        }

        AddChild(id);
    }

    cSymbol* GetName()          { return (cSymbol*)GetChild(0); }
    cDecl* GetDecl()            { return GetName()->GetDecl(); }
    virtual cTypeDecl *GetType(){ return GetDecl()->GetType(); }
    virtual bool IsStruct()     { return GetType()->IsStruct(); }
    virtual bool IsArray()      { return GetType()->IsArray(); }
    virtual bool IsPointer()    { return GetType()->IsPointer(); }
    virtual bool IsFunc()       { return GetType()->IsFunc(); }
    virtual bool IsArrayRef()   { return false; }
    virtual bool IsConst()      { return GetDecl()->IsConst(); }
    virtual int ConstValue()
    {
        if (GetDecl()->IsVar())
        {
            cVarDecl *var = GetDecl()->GetVar();
            if (var->IsConst() && (var->GetInit() != nullptr))
            {
                return var->GetInit()->ConstValue();
            }
        }
        //else if (GetDecl()->IsFunc()) { }

        return 0;
    }

    virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }
};

