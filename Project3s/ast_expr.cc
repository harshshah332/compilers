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
           //    printf("looking for ");
   	     //  puts(vid); printf("\n");
              Decl* before = NULL;
 	      before = Node::symtab->searchAllScopes(vid);		
   
              if(before == NULL){
                 ReportError::IdentifierNotDeclared(id, LookingForVariable);                  
              }
       	     else{

 	       VarDecl *vd = dynamic_cast<VarDecl *>(before);
	       this->type = vd->GetType();
  	     }	

         }
} 

//needs to be implemented 
void ConditionalExpr::Check(){   
//printf("in conditionalexpr\n");
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

//added error type
void ArithmeticExpr::Check(){
//  printf("sdfsdf");
 if(left != NULL) { 
   left -> Check();
   right -> Check();
//printf(" in arithmetic");

if ( ( left->getType() -> IsConvertibleTo(right->getType()) == false) || left->getType() == Type::voidType || left->getType() == Type::boolType  ) {

this ->type = Type::errorType;

 ReportError::IncompatibleOperands(op, left->getType(), right->getType());

}
else{
this->type = left->getType();
}

 /*
   if( (left->getType() == Type::boolType) || (left->getType() == Type::voidType) || (right->getType() == Type::boolType) || (right->getType() == Type::boolType) ) { 
	this->type = Type::errorType;
	 ReportError::IncompatibleOperands(op, left->getType(), right->getType()) ;
      }  */
}  

else{

  right -> Check();
//printf(" in arithmetic right");
  

   if( (right->getType() == Type::boolType) || (right->getType() == Type::boolType) ) { 
	this->type = Type::errorType;
	 ReportError::IncompatibleOperand(op, right->getType()) ;
      } 
}  
  
}


//added error type
void RelationalExpr::Check(){
  
   left -> Check();
   right -> Check();
// printf("in relatinalexpr");
   
  if( ( (left->getType() == Type::intType) || (left->getType() == Type::floatType) || (right->getType() == Type::intType) || (right->getType() == Type::floatType) ) && ( left->getType()->IsConvertibleTo(right->getType()) )  ) { 
	this->type = Type::boolType;
//	printf("bool");
	
	}
	else {
//	printf("heter");
	 this->type = Type::errorType;
	 ReportError::IncompatibleOperands(op, left->getType(), right->getType()) ;
      }     
  
}

//added error type
void EqualityExpr::Check(){ 

left->Check();
right->Check();

//printf("in equalityexpr\n");
   if( (left->getType()->IsConvertibleTo(right->getType())) ){
	if(left->getType() == Type::errorType){
	  this->type = Type::errorType;
 	} else {
 	this->type = Type::boolType;    
	}
    }

       else {
	    this->type = Type::errorType;
            ReportError::IncompatibleOperands(op, left->getType(), right->getType());
       }
  } 

//added errortype
void LogicalExpr::Check(){
   
left->Check();
right->Check();
//printf("in logical\n");

if( (left->getType() != Type::boolType) || (right->getType() != Type::boolType) ) {
	this->type = Type::errorType;
         ReportError::IncompatibleOperands(op, left->getType(), right->getType() );
      }
else {
  this->type = Type::boolType;
}
        
 
}


//added the error check
void AssignExpr::Check(){ 
 //  printf("in assign \n");  
left->Check();
right->Check();
    if( left->getType() != NULL && right->getType() != NULL){
    if( ( right->getType()->IsConvertibleTo(left->getType()) )  ) {
	this->type = left->getType();
//printf("about to return");
      return; 
    }    
      else {
       //	printf("here");	
          VarExpr *v = dynamic_cast<VarExpr*>(left);
	  //ReportError::InvalidInitialization(v->GetIdentifier(),  left->getType(), right->getType());
	 this->type = Type::errorType;
	 ReportError::IncompatibleOperands(op, left->getType(), right->getType() );
      }     
     }
} 



//needs to be checked 
void PostfixExpr::Check(){ 
 //  printf("in postfix \n");  
left->Check();
   

    if( left->getType() == Type::boolType || left->getType() == Type::voidType  ) {
	  this->type = Type::errorType;
          ReportError::IncompatibleOperand(op, left->getType());
    }
    
	else {
	this->type = left->getType();
	}
}



ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
 /*
//needs to be implemented
const char *ArrayAccess::getNameType(){
	return NULL;
}

//needs to be implemented
Type *ArrayAccess::getType(){
	return NULL;
} */

//needs to be implemented 
void ArrayAccess::Check(){
        
	base->Check();

        Identifier *id = dynamic_cast<VarExpr*>(base)->GetIdentifier();
        Type* elemType = NULL;
        Decl* d = Node::symtab->searchAllScopes(id->GetName());
        ArrayType* ar = NULL;       
	VarDecl* v = NULL;

	if(d ==NULL) {
           ReportError::IdentifierNotDeclared(id, LookingForVariable);
	   return;
	}
  
           else{	
	    d->Check();
            v = dynamic_cast<VarDecl*>(d);
	    if(v == NULL) {
              ReportError::NotAnArray(id);
	    }
          
	   elemType = v->GetType();
           ar = dynamic_cast<ArrayType*>(elemType);

	   if(ar == NULL){
              ReportError::NotAnArray(id);
	   } 

	   else {
              if(ar->GetElemType() == Type::intType || ar->GetElemType() == Type::boolType || ar->GetElemType() == Type::floatType)
	      {
	        return;
	      }
	   }
	   
}
 
 }


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
void FieldAccess::Check(){

base -> Check();

 Decl* d = Node::symtab->searchAllScopes(dynamic_cast<VarExpr*>(base)->GetIdentifier() ->GetName());
 Type* t = dynamic_cast<VarExpr*>(base)->getType();
 char* fieldS = NULL;

 d->Check();
 if(d == NULL) {
  ReportError::IdentifierNotDeclared(dynamic_cast<VarExpr*>(base) ->GetIdentifier(), LookingForVariable);
 }
if (t != NULL) {
 if(!t->IsVector()) {
     ReportError::InaccessibleSwizzle(field, base);
 }

 fieldS = field->GetName();
 std::string fieldString(fieldS);

if(fieldString.size() == 1) {
    return;
}
 if(fieldString.size() > 4) {
   ReportError::OversizedVector(field, base);
  }
 if(t->IsVector()) {
 if(t == Type::vec2Type) {
   if((fieldString.size() > 1 && fieldString.size() < 5)){

   if(fieldString.find('x') != std::string::npos || fieldString.find('y') != std::string::npos){
       if(fieldString.find('z') != std::string::npos || (fieldString.find('w') != std::string::npos)) {
          ReportError::SwizzleOutOfBound(field, base);
       }
       else{ return;}

   }
   else { ReportError::InvalidSwizzle(field, base);}
   }    
   }

   if(t == Type::vec3Type) {
      if(fieldString.size() > 1 && fieldString.size() < 5) {
         if( (fieldString.find('x') != std::string::npos) || (fieldString.find('y') != std::string::npos) || (fieldString.find('z') != std::string::npos)) {
              if(fieldString.find('w') != std::string::npos) { ReportError::SwizzleOutOfBound(field, base);}
	      else {return;}
	 }

	 else {ReportError::InvalidSwizzle(field, base);}
      }
   }

   if(t == Type::vec4Type) {
      if(fieldString.size() > 1 && fieldString.size() < 5) {
       if( (fieldString.find('x') != std::string::npos) || (fieldString.find('y') != std::string::npos) || (fieldString.find('z') != std::string::npos) || (fieldString.find('w') != std::string::npos)) {
          return;
       
       }

       else {ReportError::InvalidSwizzle(field, base);}
     }
    
   }
}
 }
 if(t->IsNumeric()) {ReportError::InvalidSwizzle(field, base);}
 }



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

//seems to work, possible check for call with errorType
void Call::Check(){ 
//printf("here");
Decl* d = Node::symtab->searchAllScopes(field->GetName());
FnDecl *fn = NULL;

if(d==NULL){
  ReportError::IdentifierNotDeclared(field, LookingForFunction);
  return;
}
else{
fn = dynamic_cast<FnDecl*>(d);
 if(fn==NULL){
  ReportError::NotAFunction(field);
 // return;
} 
//printf("here3\n");
else if( actuals->NumElements() > fn->GetFormals()->NumElements() )
{
  this->type = fn->GetType();
  ReportError::ExtraFormals(field, fn->GetFormals()->NumElements(),  actuals->NumElements());
  //return;
}


//printf("here4\n");
else if( actuals->NumElements() < fn->GetFormals()->NumElements() )
{
  this->type = fn->GetType();   
  ReportError::LessFormals(field, fn->GetFormals()->NumElements(), actuals->NumElements());
//  return;
}
else{
   this->type = fn->GetType();
  
  //  printf("Actuals elements = %d\n", actuals->NumElements());
      for ( int i = 0; i < actuals->NumElements(); i++ ) {
//	printf("Here in for\n");
        Expr *e = actuals->Nth(i);
	e->Check();
//	printf("formals size is %d, we are at index %d\n", fn->GetFormals()->NumElements(), i);
	VarDecl *vd = fn->GetFormals()->Nth(i);
	//vd->Check();
	if( (e->getType() -> IsConvertibleTo(vd->GetType()) ) == false){
  	  ReportError::FormalsTypeMismatch(field, i+1, vd->GetType(),  e->getType());
	  return;
	}
      } 
   }
  }
} 
