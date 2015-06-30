#pragma once

#include <string>

#include "cTypeDecl.h"
#include "cSymbol.h"
#include "cSymbolTable.h"
#include "../interp/machine_def.h"

class cArrayType : public cTypeDecl
{
  public:
    cArrayType(cSymbol *name, int size) : cTypeDecl(name, WORD_SIZE)
    {
        name = symbolTableRoot->InsertRoot(name);
        name->SetDecl(this);
        mSize = size;
    }

    virtual cTypeDecl *ParentType() 
    {
        std::string name = GetSymbol()->Name();

        // remove the last "[]"
        while (name.size() > 0 && name.back() != '[')
        {
            name.pop_back();
        }
        if (name.back() == '[') name.pop_back();

        cSymbol *sym = symbolTableRoot->Lookup(name);
        if (sym == NULL) fatal_error("Array type with no base type.");

        return dynamic_cast<cTypeDecl*>(sym->GetDecl());
    }

    virtual bool IsArray()      { return true; }
    virtual int  Size()         { return ParentType()->Size() * mSize; }
    virtual int  ElementSize()  { return ParentType()->Size(); }

    virtual std::string toString()
    {
        return "type: " + mName->toString();
    }

    virtual void GenerateCode()
    {}

    static cArrayType *ArrayType(cTypeDecl *base, int size)
    {
        std::string name;

        if (base->IsArray())
        {
            ArrayType(base->ParentType(), size);

            std::string basename = base->GetSymbol()->Name();
            name = basename.insert(basename.find("["), 
                    "[" + std::to_string(size) + "]");
            size = base->cTypeDecl::Size();
        }
        else
        {
            name = base->GetSymbol()->Name() + 
                "[" + std::to_string(size) + "]";
        }

        cSymbol *sym = symbolTableRoot->Lookup(name);
        if (sym == NULL) 
        {
            sym = new cSymbol(name);
            return new cArrayType(sym, size);
        } else {
            return dynamic_cast<cArrayType *>(sym->GetDecl());
        }

        return NULL;
    }
  protected:
};

