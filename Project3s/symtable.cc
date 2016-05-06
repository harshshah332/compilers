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
	printf("in symtab searchcurscope, its not empty, level is %d", level);
	printf("\n");
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

        if (!vec.empty()) { //check if vector is empty, no scopes
  std::vector< map < string, Decl*> >::iterator it = vec.begin();
          for (it ; it != vec.end(); ++it){
            

    //std::map <string, Decl*>::iterator search =  *it->find(searchID);
    std::pair< string, Decl*> search =  *it->find(searchID);

            if(search.second != NULL ){  //THIS IS WRONG, SECOND is not null, need to check if null
              return search.second;
            }
            else{
              return NULL;
            }


          }

        }

        return NULL;

    }


    // Add a new declared variable to current scope
        //might have to change this to search through a specific array element
    void SymbolTable::add(char* id, Decl* decl) {

      std::string searchID(id);
      if (vec.size() > 0) {
          vec.front().insert (std::pair<string, Decl*>(searchID, decl));
         } 
    }

   std::map<string, Decl*>  SymbolTable::getCurrentScope(){
  printf("in symtab get curr scope. level is %d\n", level);
printf("\n");
     return vec.at(level);
   } 
