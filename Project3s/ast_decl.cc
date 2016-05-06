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

printf("test");
	std::map<string, Decl*> curScope = Node::symtab->getCurrentScope();
	Decl* before = Node::symtab -> searchCurScope(this->GetIdentifier()->GetName()); 

	if ( before == NULL ) {

		if (this->type == assignTo->getType()) {
			curScope.insert( std::pair<string, Decl*>(this->GetIdentifier()->GetName(), this) );
		}
		else{
			ReportError::InvalidInitialization(this->GetIdentifier(), type, assignTo->getType());  
			//report error that type does not match assignto
		}

	}
	else{
		ReportError::DeclConflict(this, before);  
	}

}



void FnDecl::Check(){

    if ( formals->NumElements() > 0 ) {
	std::map<string, Decl*> fnDeclScope;
	Node::symtab->push(fnDeclScope);
      for ( int i = 0; i < formals->NumElements(); ++i ) {
          VarDecl *vd = formals->Nth(i);
          char *decName = vd->GetIdentifier()->GetName();
          
          if(decName) {
              
              Decl* before = Node::symtab->searchCurScope(decName);
              if(before != NULL){
                  ReportError::DeclConflict(vd, before);     
              }
              else{
		  fnDeclScope.insert(std::pair<string, Decl*>(decName, vd));
                                   
              }
          }
      }
          
        
    }
   if(body){   
	body->Check();
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

