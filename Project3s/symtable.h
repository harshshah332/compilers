/**
 * File: symtable.h
 * ----------- 
 *  Header file for Symbol table implementation.
 */

#include <map>
#include <string.h>
#include <vector>
using namespace std;


class SymbolTable { 

public: 
	vector<map<string, Decl*>> vec; 

 	int level;

 	SymbolTable();
    SymbolTable(map<string, Decl*>  *map, int lvl);
    void Push();
    void remove();
    Decl* SearchHead(char* id);
  	Decl* Search(char* id);
	void Add(char* id, Decl* decl);


}; 
