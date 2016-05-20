/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
#include "errors.h"
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && tq != NULL);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL && tq != NULL);
    (type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}


void VarDecl::Check(){

//printf(" \ntest in var decl \n");
	//call symtab->searchCurScope, which searches the current scope, we can declare a 
	//variable in method with same as in any scope except current scope
	Decl* before = Node::symtab -> searchCurScope(this->GetIdentifier()->GetName());
//	puts(  this->GetIdentifier()->GetName());

//	printf("\n");

	if ( before != NULL ) {
	 ReportError::DeclConflict(this, before);
	} 
	else{ 
 	   Node::symtab -> insertCurScope(this->GetIdentifier()->GetName(), this) ;	
	}


           if(this->assignTo != NULL) {
		assignTo->Check();
               
/*	if( assignTo->getType() == Type::errorType){
		printf("no\n");*/ 


                  if (assignTo->getType()->IsConvertibleTo(this->type) != true) { //  this->type != assignTo->getType() && assignTo->getType() != Type::errorType) { //printf("in init\n");
			 ReportError::InvalidInitialization(this->GetIdentifier(), this->type, assignTo->getType());  
			//report error that type does not match assignto
	    	   }
	    	
}

//https://piazza.com/class/ilg2qlo1ijg10f?cid=790
}



void FnDecl::Check(){

Program::returnExist = 0;

   if(returnType != NULL){
	Program::fnReturnType = returnType;
//printf("the return type is ");
//puts(Program::fnReturnType->getNameType());
//printf("\n");
}

       char * decName = this->GetIdentifier()->GetName();
          if(decName) {
		string t = std::string(decName);
//               printf("fnName trying to be added is %s\n",t.c_str());
              Decl* before = Node::symtab->searchCurScope(decName);


              if(before != NULL){
                  ReportError::DeclConflict(this, before);
                  
              }
              else{  // printf("yee decname is not there before\n");
                  Node::symtab->insertCurScope(decName, this);
                  
              }
          } 



   if(body){   
//printf("in fndecl. formals size is %d\n", static_cast<int>(formals->NumElements()));
        StmtBlock *b = dynamic_cast<StmtBlock*>(body);
	b->Check(formals);
       // b->Check();
   }

if (Program::returnExist == 0 && returnType != Type::voidType) {
ReportError::ReturnMissing(this);
} 
   
}   


FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}

