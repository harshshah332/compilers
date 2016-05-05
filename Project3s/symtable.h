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

    void push(map<string, Decl*> temp);
    void remove();
    Decl *searchHead(char* id);
  	Decl *search(char* id);
	void add(char* id, Decl* decl);


}; 
