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
using namespace std;





  

    SymbolTable::SymbolTable() { 
        level = 0; 
    }


    // Adds element to list end
    // Call this whenever we go int 
    void SymbolTable::push(map<string, Decl*> temp)
    { 
      vec.push_back(temp);
      level++;
    }



    // Removes head
    void SymbolTable::remove()
    { 
      int s;
    }



    // Checks if id exists in current scope 
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

    // Find the location of the id in the nearest scope
    // if not found returns NULL


    
    Decl* SymbolTable::search(char* id) {
      std::string searchID(id);

        if (vec.size() > 0) { //check if vector is empty, no scopes
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

   
