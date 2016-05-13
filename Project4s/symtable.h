/**
 * File: symtable.h
 * ----------- 
 *  Header file for Symbol table implementation.
 */

#include <map>
#include <string.h>
#include "irgen.h"
#include "list.h"
#include <vector>
#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"

 class Decl;
using namespace std;

class Symbol {

    
public:
    Symbol() {}
    map<string, llvm::Value*> sym_map;
    void insertSymbol(char*, llvm::Value*);
    llvm::Value* getSymbol(char*) const;
    bool exists(char*) const;
};





class SymbolTable { 

public: 


	List< Symbol* > *symtab_list;
    
    SymbolTable() { symtab_list = new List<Symbol*>(); }
    Symbol* getCurScope(){ return symtab_list->Nth(symtab_list->NumElements()-1); }
    Symbol* getGScope(){ return symtab_list->Nth(0); }
    Symbol* getScope(int i) { return symtab_list->Nth(i); }
    int curLevel(){ return symtab_list->NumElements()-1; }
    void insertScope() { symtab_list->Append(new Symbol()); }
    void deleteScope() { symtab_list->RemoveAt(symtab_list->NumElements()-1); }



}; 

