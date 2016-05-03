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





 

 public:
    SymbolTable() { 
       	vec = new map<string, Decl*> ();
        level = 0; 
        parent = NULL;
    }

    SymbolTable(map<string, Decl*>  map, int lvl) {
    	level = 0; 
    	vec = new map<string, Decl*> ();
    	vec.push_back(map); 
        level = lvl; 
    }

    // Adds element to list end
    // Call this whenever we go int 
    void Push()
    { 
      map<string, Decl*> temp = new map<string, Decl*>();
      vec.push_back(temp);
      level++;
    }



    // Removes head
    void remove()
    { 
      int s;
    }



    // Checks if id exists in current scope 
    Decl* SearchHead(char* id) {

    	std::string searchID(id);
    //might have to change this to search through a specific array element
      	if (vec){

      	auto search =  vec.front().find(searchID);

      	if(search != vec.front().end){
      		return search->second;
      	}
      	else{
      		return NULL;
      	}

      }
    }

    // Find the location of the id in the nearest scope
    // if not found returns NULL


    
    Decl* Search(char* id) {
    	std::string searchID(id);

        if (vec) {

       		for (std::vector<map<string, Vardecl*>>::iterator it = myvector.begin() ; it != myvector.end(); ++it){
       			
       			auto search =  *it->find(searchID);

       			if(search != *it->end){
      				return search->second;
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
    void Add(char* id, Decl* decl) {

    	std::string searchID(id);
     	if (vec) {
      		vec.front().insert (std::pair<string, Decl*>(searchID, decl));
  	   	 } 
    }

   