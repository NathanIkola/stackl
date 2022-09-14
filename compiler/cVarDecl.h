#pragma once

#include <string>

#include "cDecl.h"
#include "cArrayType.h"
#include "cExpr.h"
#include "cSymbol.h"

class cVarDecl : public cDecl
{
  public:
    cVarDecl(cTypeDecl *type, cSymbol *name, bool isStruct=false) : cDecl()
    {
        if (symbolTableRoot->LocalLookup(name->Name()) != nullptr)
        {
            ThrowSemanticError(name->Name() +" already defined in local scope");
            return;
        }

        // FIX THIS: what about 2-star pointers?
        if (isStruct)
        {
            if (!type->IsCompleteType())
            {
                ThrowSemanticError(name->Name() + " is not a complete type");
                return;
            }

            if ((type->IsPointer() && (cPointerType*)type->ParentType()->IsStruct()) ||
                 type->IsStruct())
            {}
            else
            {
                ThrowSemanticError(name->Name() +" Struct decl not of type Struct");
                return;
            }
        }
        
        // if the symbol exists in an outer scope, we need to create a new one
        // instead of re-using the outer scoped symbol
        if (symbolTableRoot->Lookup(name->Name()) != nullptr)
        {
            name = new cSymbol(name->Name());
        }

        name->SetDecl(this);
        symbolTableRoot->Insert(name);

        AddChild(type);
        AddChild(name);
        AddChild(nullptr);  // array size
        AddChild(nullptr);  // init Expr
        mOffset     = 0;
        mIsGlobal   = false;
        mHasInit    = false;
        mIsConst    = false;
        mIsStatic   = false;
        mIsExtern   = false;
    }

    cVarDecl(cVarDecl *base, cExpr *arraySize) : cDecl()
    {
        if (!arraySize->IsConst())
        {
            ThrowSemanticError("Array dimension must be a const");
            return;
        }

        int size = arraySize->ConstValue();
        AddChild(new cArrayType(base->GetType(), size));
        AddChild(base->GetName());
        AddChild(arraySize);
        AddChild(nullptr);      // init expr
        
        base->GetName()->SetDecl(this);

        mOffset     = 0;
        mIsGlobal   = false;
        mHasInit    = false;
        mIsConst    = true;
        mIsStatic   = false;
        mIsExtern   = false;
    }

    void SetInit(cExpr *init) 
    { 
        if (GetType()->IsArray() || GetType()->IsStruct())
        {
            ThrowSemanticError("Cannot initialize arrays or structs");
            return;
        }
        SetChild(3, init); 
        mHasInit = true;
    }
    void SetGlobal()        { mIsGlobal = true; }
    void SetConst()         { mIsConst = true; }
    void SetStatic()         { mIsStatic = true; }
    void SetExtern()         { mIsExtern = true; }

    virtual bool IsGlobal() { return mIsGlobal; }
    virtual bool IsStatic() { return mIsStatic; }
    virtual bool IsExtern() { return mIsExtern; }
    virtual bool IsConst()
    {
        if (GetType()->IsArray())
            return mIsConst;
        return mIsConst && HasInit() && GetInit()->IsConst();
    }

    virtual bool IsVar()    { return true; }

    virtual void SetOffset(int offset)  { mOffset = offset; }
    virtual int  GetOffset()            { return mOffset; }

    virtual cTypeDecl* GetType()    
    { 
        cTypeDecl* type = (cTypeDecl*)GetChild(0);

        if (mIsConst && HasInit() && type->IsInt())
            return GetInit()->GetType();
        else
            return (cTypeDecl*)GetChild(0); 
    }
    virtual std::string GetVarName()   
    { 
        std::string name = GetName()->Name();
        if (IsGlobal() && IsStatic())
            return "$" + name;
        else
            return name;
    }
    virtual cSymbol* GetName()      { return (cSymbol*)GetChild(1); }
    virtual cExpr*   GetArraySize() { return (cExpr*)GetChild(2); }
    virtual cExpr*   GetInit()      { return (cExpr*)GetChild(3); }
    bool             HasInit()      { return mHasInit; }

    virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }
  protected:
    int       mOffset;
    bool      mIsGlobal;
    bool      mHasInit;
    bool      mIsConst;
    bool      mIsStatic;
    bool      mIsExtern;
};

