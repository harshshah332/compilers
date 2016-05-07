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





  

    SymbolTable::SymbolTable() { 
//printf("test");
        level = -1; 
    }


    // Adds element to list end
    // Call this whenever we go int 
    void SymbolTable::push(map<string, Decl*> temp)
    { 
      vec.push_back(temp);
      level++;
//printf("level");
    }



    // Removes head
    void SymbolTable::popBack()
    { 
      if(!vec.empty()){
	vec.pop_back();
	level = level-1;
      }
    }



    // Checks if id exists in head scope 
    Decl* SymbolTable::searchHead(char* id) {

      std::string searchID(id);
    //might have to change this to search through a specific array element
        if (vec.size() > 0){ //check is vector is empty, no scopes

  std::map <string, Decl*>::iterator it;
        it =  vec.front().find(searchID);

        if(it != vec.front().end() ){
          return it->second;
        }
  else {
    return NULL;
  } 

      }
        else{
          return NULL;
        }
    }


    // Checks if id exists in current scope 
    Decl* SymbolTable::searchCurScope(char* id) {

      std::string searchID(id);
        if (!vec.empty()){ //check is vector is empty, no scopes
int x = 4;
//	printf("in symtab searchcurscope, its not empty, level is %d. Searching for %s \n", level, searchID.c_str());
//printf("the size of the map at this level is %d", static_cast<int>(vec.at(level).size()));
  	  std::map <string, Decl*>::iterator it;
          it =  vec.at(level).find(searchID);

          if(it != vec.at(level).end() ){
            return it->second;
          }
         else {
           return NULL;
         } 
 
      }
      else{
        return NULL;
      }
}





    // Find the location of the id in the nearest scope
    // if not found returns NULL

    Decl* SymbolTable::search(char* id) {
      std::string searchID(id);

     if (!vec.empty() && vec.size() > 0) { //check if vector is empty, no scopes
  	int curLevel = level;
	while( curLevel >= 1 ) {
  	  std::map <string, Decl*>::iterator it;
          it =  vec.at(curLevel).find(searchID);

          if(it != vec.at(curLevel).end() ){
            return it->second;
          }
      	  curLevel = curLevel -1; 
          
	}
 	return NULL;
   }
   else {
     return NULL;
      }
}
	   


    void SymbolTable::insertCurScope(char* id, Decl* decl) {

      std::string searchID(id);
      if (vec.size() > 0) {
          vec.at(level).insert (std::pair<string, Decl*>(searchID, decl));
         } 
    }

   std::map<string, Decl*>  SymbolTable::getCurrentScope(){
//  printf("in symtab get curr scope. level is %d\n", level);

     return vec.at(level);
   } 
