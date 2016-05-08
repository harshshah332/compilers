/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"
#include "errors.h"

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
    return strcmp(tokenString, op) == 0;
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void VarExpr::Check(){

          char *vid = id->GetName();
          
          if(vid) {
              
              Decl* before = NULL;
 	      before = Node::symtab->searchAllScopes(vid);		
   
              if(before == NULL){
                 ReportError::IdentifierNotDeclared(id, LookingForType);                  
              }


              }
      } 

//needs to be implemented 
void ConditionalExpr::Check(){   
printf("in conditionalexpr\n");
    cond -> Check();
    if(cond->getType() == Type::boolType ) {
          return;
      }
      else {

         ReportError::TestNotBoolean(cond);         

      }
  }



void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}

//needs to be checked 
void ArithmeticExpr::Check(){
  printf("sdfsdf");
 if(left != NULL) { 
   left -> Check();
   right -> Check();
printf(" in arithmetic");

if ( (left->getType() -> IsEquivalentTo(right->getType())) != true ) {
 ReportError::IncompatibleOperands(op, left->getType(), right->getType());
}

 
   if( (left->getType() == Type::boolType) || (left->getType() == Type::voidType) || (right->getType() == Type::boolType) || (right->getType() == Type::boolType) ) { 

	 ReportError::IncompatibleOperands(op, left->getType(), right->getType()) ;
      } 
}  

else{

  right -> Check();
printf(" in arithmetic right");
  

   if( (right->getType() == Type::boolType) || (right->getType() == Type::boolType) ) { 

	 ReportError::IncompatibleOperand(op, right->getType()) ;
      } 



}  
  
}


//needs to be checked 
void RelationalExpr::Check(){
  
   left -> Check();
   right -> Check();
printf("in relatinalexpr");
   
  if( ( (left->getType() == Type::intType) || (left->getType() == Type::floatType) || (right->getType() == Type::intType) || (right->getType() == Type::floatType) ) && ( left->getType()->IsEquivalentTo(right->getType()) )  ) { 
	return;
	}
	else {
	 ReportError::IncompatibleOperands(op, left->getType(), right->getType()) ;
      }     
  
}

//needs to be checked
void EqualityExpr::Check(){ 

left->Check();
right->Check();

printf("in equalityexpr\n");
   if( (left->getType()->IsEquivalentTo(right->getType())) ){
      return;
    }

       else {
            ReportError::IncompatibleOperands(op, left->getType(), right->getType());
       }
  } 

//needs to be checked
void LogicalExpr::Check(){
   
left->Check();
right->Check();
printf("in logical\n");

if( (left->getType() == Type::boolType) && (right->getType() == Type::boolType) ) {
         ReportError::IncompatibleOperands(op, left->getType(), right->getType() );
      }
        
 
}


//needs to be checked 
void AssignExpr::Check(){ 
   printf("in assign \n");  
left->Check();
right->Check();
    if( left->getType() != NULL && right->getType() != NULL){
    if( left->getType()->IsEquivalentTo(right->getType()) ) {
      return; 
    }    
      else {
       		
          VarExpr *v = dynamic_cast<VarExpr*>(left);
	  ReportError::InvalidInitialization(v->GetIdentifier(),  left->getType(), right->getType());
      }     
     }
} 



//needs to be checked 
void PostfixExpr::Check(){ 
   printf("in postfix \n");  
left->Check();
   

    if( left->getType() == Type::boolType || left->getType() == Type::voidType  ) {
          ReportError::IncompatibleOperand(op, left->getType());
    }
    
	else return;
}



ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
 
//needs to be implemented
const char *ArrayAccess::getNameType(){
	return NULL;
}

//needs to be implemented
Type *ArrayAccess::getType(){
	return NULL;
}

//needs to be implemented 
void ArrayAccess::Check(){ int x; } ;

FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
}

//needs to be implemented 
void FieldAccess::Check(){ int x; } ;

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::PrintChildren(int indentLevel) {
   if (base) base->Print(indentLevel+1);
   if (field) field->Print(indentLevel+1);
   if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}

//needs to be implemented 
void Call::Check(){ int x; } ;
