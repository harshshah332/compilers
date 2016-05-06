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
	printf("decl numelements is %d\n", decls->NumElements());
	std::map<string, Decl*> globalScope;
	Node::symtab->push(globalScope);
      for ( int i = 0; i < decls->NumElements(); ++i ) {
        Decl *d = decls->Nth(i);
            char *decName = d->GetIdentifier()->GetName();
          
          if(decName) {
		string t = std::string(decName);
              printf("Decname trying to be added is %s\n",t.c_str());
              Decl* before = Node::symtab->searchCurScope(decName);
	 //  printf("name of before is %s\n", std::string(before->GetIdentifier()->GetName()).c_str());

              if(before != NULL){
                  ReportError::DeclConflict(d, before);
                  
              }
              else{ printf("yee decname is not there before\n");
                  Node::symtab->add(decName, d);
                  
              }
          }
      }
           for (int i = 0; i < decls->NumElements(); ++i){
            this->decls->Nth(i)->Check();
        }

printf("the size on starting is %d\n", static_cast<int>(Node::symtab->getCurrentScope().size()) ); 
    

        /* !!! YOUR CODE HERE !!!
         * Basically you have to make sure that each declaration is 
         * semantically correct.
         */
        
    }
}

//do we declare each new map scope in the check function or do we create it as a
//variable in the .h and use that???
void StmtBlock::Check(){

printf("in stmtbLOCK\n");
	//printf("stmts numelements is %d\n", stmts->NumElements());

    if ( stmts->NumElements() > 0 ){
	printf("stmts numelements is %d\n", stmts->NumElements());
     Node *parent = this->GetParent();  //get the parent of this stmt block
     if ( dynamic_cast<StmtBlock *>(parent) != NULL){ 
    //if the parent is a stmtblock, then we can create a new scope 
      std::map <string,Decl* > stmtScope;
printf("parent is stmtblock\n");
      Node::symtab->push(stmtScope); 

    /* 
      for ( int i = 0; i < decls->NumElements(); ++i ) {
          VarDecl *vd = decls->Nth(i);
          char *decName = vd->GetIdentifier()->GetName();
          
          if(decName) {
              
              Decl* before = NULL;
 	      std::map <string, Decl*>::iterator it = stmtScope.find(decName);

	      if(it != stmtScope.end()){
	          before = it->second;
	      }

              if(before != NULL){
                  ReportError::DeclConflict(vd, before);
                  
              }
              else{
                  stmtScope.insert(std::pair<string, Decl*>(decName, vd));
              }
          }
      } */
     } 
/*
	else{
     //if the parent is not a stmtblock, then get the current scope, and add to that
          for ( int i = 0; i < decls->NumElements(); ++i ) {
            VarDecl *vd = decls->Nth(i);
            char *decName = vd->GetIdentifier()->GetName();
          
          if(decName) {
              
              Decl* before = NULL;
	      std::map <string, Decl*> curScope = Node::symtab->getCurrentScope();
	      std::map <string, Decl*>::iterator it = curScope.find(decName);

	      if(it != curScope.end()){
	          before = it->second;
	      }

              if(before != NULL){
                  ReportError::DeclConflict(vd, before);
                  
              }
              else{
                  curScope.insert(std::pair<string, Decl*>(decName, vd));
              }
           }
          }
	 
	}
 */
	//now pop the last scope. This would be the newly inserted stmtBlock scope or the scope of the
	//stmtBlock from which is was created, ex) " if() { ... } " pop the if
        for (int i = 0; i < stmts->NumElements(); ++i){
	 //   Stmt *st = stmts->Nth(i);
	 //   st->Check();
	 printf("here\n");
            this->stmts->Nth(i)->Check();
        }
	printf("symtab size is %d\n",static_cast<int>(Node::symtab->vec.size()) );
	Node::symtab->popBack();
	printf("symtab size is %d\n",static_cast<int>(Node::symtab->vec.size()) );
	

    }
/*
    if (stmts->NumElements() > 0){
        for (int i = 0; i < stmts->NumElements(); ++i){
	 //   Stmt *st = stmts->Nth(i);
	 //   st->Check();
            this->stmts->Nth(i)->Check();
        }
    } */
  

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

//do we even need a check? confirm?
void ForStmt::Check(){

   if(init != NULL){
      init -> Check();
   }
   if(step != NULL){
      step -> Check();
   }



}



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
      if ((dynamic_cast<WhileStmt*>(parent)!=NULL) ||
          (dynamic_cast<SwitchStmt*>(parent)!=NULL) )
     //     || (dynamic_cast<ForStmt*>(parent)!=NULL) )  can we have continue inside ifStmt? confirm?
      {
       return; 
      }

      parent = parent->GetParent();
    }
  ReportError::ContinueOutsideLoop(this); 
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

//needs implementation
void WhileStmt::Check(){

int x;



} 

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

void IfStmt::Check(){
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

//needs to be implemented
void ReturnStmt::Check(){ int x; }

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
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

