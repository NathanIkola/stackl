#pragma once

#include <string>

#include "cStmt.h"
#include "cSymbol.h"

class cTypeDecl;
class cVarDecl;
class cFuncDecl;
class cDecl : public cStmt
{
  public:
    cDecl() : cStmt() {}

    virtual bool IsStatic() { return false; }
    virtual bool IsType()   { return false; }
    virtual bool IsVar()    { return false; }
    virtual bool IsFunc()   { return false; }
    virtual bool IsStruct() { return false; }
    virtual bool IsPointer(){ return false; }
    virtual bool IsArray()  { return false; }
    virtual bool IsInt()    { return false; }
    virtual bool IsConst()  { return false; }
    virtual cSymbol *GetName()      = 0;
    //virtual int GetOffset() = 0;
    virtual cTypeDecl *GetType();
    virtual cVarDecl  *GetVar();
    virtual cFuncDecl *GetFunc();

    virtual void Visit(cVisitor *visitor) { visitor->Visit(this); }

};
