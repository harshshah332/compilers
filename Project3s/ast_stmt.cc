/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "symtable.h"

//SymbolTable *Program::symtab = new SymbolTable();

Type *Program::fnReturnType = NULL;
int Program::returnExist = 0;
//FnDecl *Program::funcDecl = NULL;

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */

    // sample test - not the actual working code
    // replace it with your own implementation
    if ( decls->NumElements() > 0 ) {
//	printf("decl numelements is %d\n", decls->NumElements());
	std::map<string, Decl*> globalScope;
	Node::symtab->push(globalScope);
      for ( int i = 0; i < decls->NumElements(); ++i ) {
        Decl *d = decls->Nth(i);
            char *decName = d->GetIdentifier()->GetName();
      /*    
          if(decName) {
		string t = std::string(decName);
//               printf("Decname trying to be added is %s\n",t.c_str());
              Decl* before = Node::symtab->searchCurScope(decName);


              if(before != NULL){
                  ReportError::DeclConflict(d, before);
                  
              }
              else{  // printf("yee decname is not there before\n");
                  Node::symtab->insertCurScope(decName, d);
                  
              }
          } */
//printf("the level is %d\n",(Node::symtab->level) ); 

	  d->Check();
      }


//printf("the level is %d\n",(Node::symtab->level) ); 
 
    

        /* !!! YOUR CODE HERE !!!
         * Basically you have to make sure that each declaration is 
         * semantically correct.
         */
        
    }
}

void DeclStmt::Check(){

	if(decl !=NULL){
	decl->Check();
	}
}

//do we declare each new map scope in the check function or do we create it as a
//variable in the .h and use that???
void StmtBlock::Check(List<VarDecl*> *formals){

//printf("in stmtbLOCK\n");
	//printf("stmts numelements is %d\n", stmts->NumElements());
         std::map <string,Decl* > stmtScope;
         Node::symtab->push(stmtScope); 

if(formals!=NULL){
    if ( formals->NumElements() > 0 ){
//	printf("formals != null,  formals numelements is %d\n", formals->NumElements());

      for ( int i = 0; i < formals->NumElements(); ++i ) {
          Decl *vd = formals->Nth(i);
          char *decName = vd->GetIdentifier()->GetName();
          
          if(decName) {
              
              Decl* before = NULL;
 	      std::map <string, Decl*>::iterator it = symtab->getCurrentScope().find(decName);

	      if(it != symtab->getCurrentScope().end()){
	          before = it->second;
	      }

              if(before != NULL){
                  ReportError::DeclConflict(vd, before);
                  
              }
              else{
                  symtab->insertCurScope(decName, vd);
//	printf("just for test\n");
              }
          }
      } 
     } 



}

    if ( stmts->NumElements() > 0 ){
//	printf("stmts numelements is %d\n", stmts->NumElements());
//	printf("cur level is %d\n", Node::symtab->level);
      for ( int i = 0; i < stmts->NumElements(); ++i ) {
//printf("in for loop. I is %d\n", i);

          Stmt *st = stmts->Nth(i);
	  st->Check();

//printf("return from for. size of the map at level %d is %d \n",Node::symtab->level,  static_cast<int>(Node::symtab->vec.at(1).size()));

      } 
     } 

//	printf("symtab size is %d\n",static_cast<int>(Node::symtab->vec.size()) );
	Node::symtab->popBack();
//	printf("symtab size is %d\n",static_cast<int>(Node::symtab->vec.size()) );
	

    }

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
    // Node::symtab->push(stmtScope);
   }

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this); 
}

/* //we dont need a check for this, according to piazza
void ConditionalStmt::Check() {  
    test->Check(); //call check on the test expr
    if(strcmp(test-> getNameType(), "bool")){
      ReportError::TestNotBoolean(test);
    }

    body->Check();
}
*/



void BreakStmt::Check() {
  Node *parent = this->GetParent();
  while (parent)
    {
      if ((dynamic_cast<WhileStmt*>(parent)!=NULL) ||
          (dynamic_cast<SwitchStmt*>(parent)!=NULL)  ||
          (dynamic_cast<ForStmt*>(parent)!=NULL) ) {
       return; 
      }

      parent = parent->GetParent();
    }
  ReportError::BreakOutsideLoop(this); 
}

void ContinueStmt::Check(){
  Node *parent = this->GetParent();
  while (parent)
    {
      if ((dynamic_cast<WhileStmt*>(parent)!=NULL) 
     //     (dynamic_cast<SwitchStmt*>(parent)!=NULL) ) can we have continue inside ifStmt? confirm?
         || (dynamic_cast<ForStmt*>(parent)!=NULL) )     
       {
       return; 
      }

      parent = parent->GetParent();
    }
  ReportError::ContinueOutsideLoop(this); 
}



//do we even need a check? confirm?
void ForStmt::Check(){

   if(init != NULL){
      init -> Check();
   }

   test->Check();

   if( test->getType() ->IsConvertibleTo( Type::boolType) == false ) {	
	ReportError::TestNotBoolean (test);
   }

   if(step != NULL){
      step -> Check();
   }

   if(body != NULL){
      body -> Check();
   }
    
}


ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && b != NULL);
    (init=i)->SetParent(this);
    step = s;
    if ( s )
      (step=s)->SetParent(this);

}



void ForStmt::PrintChildren(int indentLevel) {
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    if ( step )
      step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

void WhileStmt::Check(){
	
	test->Check();
	
   if( test->getType()->IsConvertibleTo(Type::boolType) == false ) {	
	ReportError::TestNotBoolean (test);
   }

  if (body != NULL) {
    
      body -> Check();
   }


} 

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

void IfStmt::Check(){
 
test->Check(); 
 if( test->getType()->IsConvertibleTo(Type::boolType) == false ) {	
	ReportError::TestNotBoolean (test);
   }


    if(body != NULL) {
    body->Check();
   }

    if(elseBody != NULL){
      elseBody -> Check();
    }


}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    if (test) test->Print(indentLevel+1, "(test) ");
    if (body) body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

// returns a seg fault, the getNameType returns null
void ReturnStmt::Check(){

//Boolean *t = true;
//int * t = 1;
Program::returnExist = 1;
    Type *given = NULL;
    if(expr == NULL) { 
    given = Type::voidType;
//	printf("given is null, so set as void");
    } else {
	expr->Check();
    given =  expr->getType();
    }
VarDecl *returnDecl = dynamic_cast<VarDecl*>(given);
//if the return type is a varexpr, then check if that varexpr exists in current scope, 
//if not, then return not declared error, else check if its type matches with return, and 
//report error or return
if(returnDecl != NULL){
   if( (Node::symtab->searchCurScope(returnDecl->GetIdentifier()->GetName()) == NULL) || (Node::symtab->searchHead(returnDecl->GetIdentifier()->GetName()) == NULL)  ){
	ReportError::IdentifierNotDeclared(returnDecl->GetIdentifier(), LookingForVariable);
   }
     else if ( given->IsConvertibleTo( Program::fnReturnType) != true ) { //if they are not the same type,return error, else return
       ReportError::ReturnMismatch(this, given, Program::fnReturnType);  //else report error
     }
  
     else{
 
	return;    
     }
}

//if the return type is not a varexpr, then check if its type matches with return, and
//report error or return
   if ( given->IsConvertibleTo( Program::fnReturnType) ) { //if they are the same type,return
     return; 
   }
  
   else{
 
     ReportError::ReturnMismatch(this, given, Program::fnReturnType);  //else report error
   }
 
   

}   


void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
}

void SwitchStmt::Check() {

   if( expr != NULL){
	expr -> Check();
   }

   if( cases != NULL){
      for ( int i = 0; i < cases->NumElements(); ++i ) {
	cases->Nth(i)->Check();
      }
   }

   if( def != NULL){
        def -> Check();
   }


}


void Case::Check() {

   if (label != NULL){
	label->Check();
   }

   if (stmt != NULL){
	stmt -> Check();
   }
}

void Default::Check() {

   if (stmt != NULL){
	stmt -> Check();
   }
}



SwitchLabel::SwitchLabel(Expr *l, Stmt *s) {
    Assert(l != NULL && s != NULL);
    (label=l)->SetParent(this);
    (stmt=s)->SetParent(this);
}

SwitchLabel::SwitchLabel(Stmt *s) {
    Assert(s != NULL);
    label = NULL;
    (stmt=s)->SetParent(this);
}

void SwitchLabel::PrintChildren(int indentLevel) {
    if (label) label->Print(indentLevel+1);
    if (stmt)  stmt->Print(indentLevel+1);
}

SwitchStmt::SwitchStmt(Expr *e, List<Stmt *> *c, Default *d) {
    Assert(e != NULL && c != NULL && c->NumElements() != 0 );
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
    def = d;
    if (def) def->SetParent(this);
}

void SwitchStmt::PrintChildren(int indentLevel) {
    if (expr) expr->Print(indentLevel+1);
    if (cases) cases->PrintAll(indentLevel+1);
    if (def) def->Print(indentLevel+1);
}

