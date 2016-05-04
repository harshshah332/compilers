/**
 * File: symtable.h
 * ----------- 
 *  Header file for Symbol table implementation.
 */

#include <map>
#include <string.h>
#include <vector>
#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"

 class Decl;
using namespace std;


class SymbolTable { 

public: 


	vector< map < string, Decl*> > vec; 

 	int level;

 	SymbolTable();
    SymbolTable(map < string, Decl*>  mymap, int lvl);
    void Push();
    void remove();
    Decl *SearchHead(char* id);
  	Decl *Search(char* id);
	void Add(char* id, Decl* decl);


}; 
