/*
 * Symbol table implementation
 *
 */



#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "symtable.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <stdio.h>
using namespace std;





void Symbol::insertSymbol(char* ctr, llvm::Value* val) {
    string str(ctr);
    if(exists(ctr)) {
        sym_map.at(str) = val;
    }
    else sym_map.insert(pair<string, llvm::Value*>(str, val));
}

llvm::Value* Symbol::getSymbol(char* ctr) const {
    if(exists(ctr)) {
        string str(ctr);
        return sym_map.at(str);
    }
    return NULL;
}


bool Symbol::exists(char* ctr) const {
    string str(ctr);
    return (sym_map.count(str) > 0);
}


