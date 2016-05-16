/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

llvm::Value* IntConstant::Emit() {
  return llvm::ConstantInt::get(irgen->GetIntType(), value);
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

llvm::Value* FloatConstant::Emit() {
  return llvm::ConstantFP::get(irgen->GetFloatType(), value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

llvm::Value* BoolConstant::Emit() {
  return llvm::ConstantInt::get(irgen->GetBoolType(), value);
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}


//needs to be implemented
llvm::Value* VarExpr::Emit() {
  llvm::Value *mem = NULL;

  int i;
  for(i = symtab->curLevel(); i >= 0; i--) {
    mem = symtab->at(i)->getSymbol(this->id->GetName());
    if(mem) break;
  }

  llvm::Twine *tw = new llvm::Twine(this->id->GetName());
  llvm::Value *ret = new llvm::LoadInst(mem, *tw, irgen->GetBasicBlock());

  return ret;
}

llvm::Value* VarExpr::EmitAddress() {
  llvm::Value *mem = NULL;

  int i;
  for(i = symtab->curLevel(); i >= 0; i--) {
    mem = symtab->at(i)->getSymbol(this->id->GetName());
    if(mem) break;
  }

  return mem;
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

//needs to be implemented
llvm::Value* ArithmeticExpr::Emit() {
  FieldAccess *r_fa = dynamic_cast<FieldAccess*>(right);
  FieldAccess *l_fa = NULL;
  llvm::Value *l = NULL;
  llvm::Value *r = right->Emit();
  llvm::Value *ret = NULL;
  llvm::Type *ty = r->getType();
  char *l_swizzle = NULL;
  char *r_swizzle = NULL;
  int l_swizzle_len = 0;
  int r_swizzle_len = 0;


  bool FTy = (ty != (llvm::Type*)irgen->GetIntType());

  if(left) {
    l = left->Emit();

  }

  string str = op->toString();
    if(!str.compare("+")) {

      ret = (FTy) ? llvm::BinaryOperator::CreateFAdd(l, r, "", irgen->GetBasicBlock()) : 
                    llvm::BinaryOperator::CreateAdd(l, r, "", irgen->GetBasicBlock());
    
  
  } else if(!str.compare("-")) {

      ret = (FTy) ? llvm::BinaryOperator::CreateFSub(l, r, "", irgen->GetBasicBlock()) :
                    llvm::BinaryOperator::CreateSub(l, r, "", irgen->GetBasicBlock());
  

  } else if(!str.compare("*")) {

      ret = (FTy) ? llvm::BinaryOperator::CreateFMul(l, r, "", irgen->GetBasicBlock()) :
                    llvm::BinaryOperator::CreateMul(l, r, "", irgen->GetBasicBlock());
    

  } else if(!str.compare("/")) {

      ret = (FTy) ? llvm::BinaryOperator::CreateFDiv(l, r, "", irgen->GetBasicBlock()) :
                  llvm::BinaryOperator::CreateUDiv(l, r, "", irgen->GetBasicBlock());
    

  } else if(!str.compare("++")) {
    llvm::Value *inc = (FTy) ? llvm::ConstantFP::get(irgen->GetFloatType(), 1.f) :
                               llvm::ConstantInt::get(irgen->GetIntType(), 1);


      ret = (FTy) ? llvm::BinaryOperator::CreateFAdd(r, inc, "", irgen->GetBasicBlock()) :
                    llvm::BinaryOperator::CreateAdd(r, inc, "", irgen->GetBasicBlock());

      VarExpr* r_var = dynamic_cast<VarExpr*>(right);
      new llvm::StoreInst(ret, r_var->EmitAddress(), irgen->GetBasicBlock());
    

  } else if(!str.compare("--")) {
    llvm::Value *dec = (FTy) ? llvm::ConstantFP::get(irgen->GetFloatType(), 1.f) :
                               llvm::ConstantInt::get(irgen->GetIntType(), 1);


      ret = (FTy) ? llvm::BinaryOperator::CreateFSub(r, dec, "", irgen->GetBasicBlock()) :
                  llvm::BinaryOperator::CreateSub(r, dec, "", irgen->GetBasicBlock());

      VarExpr* r_var = dynamic_cast<VarExpr*>(right);
      new llvm::StoreInst(ret, r_var->EmitAddress(), irgen->GetBasicBlock());
    
  }

  return ret;
}

//needs to be implemented
llvm::Value* PostfixExpr::Emit() {
  FieldAccess *l_fa = dynamic_cast<FieldAccess*>(left);
  VarExpr* l_var; // = dynamic_cast<VarExpr*>(left);
  llvm::Value *l_addr = NULL;
  //llvm::Value *i = llvm::ConstantInt::get(irgen->GetIntType(), 1);
  llvm::Value *l = left->Emit();
  llvm::Type *ty = l->getType();
  bool FTy = (ty != (llvm::Type*)irgen->GetIntType());
  llvm::Value *ival = (FTy) ? llvm::ConstantFP::get(irgen->GetFloatType(), 1.f) :
                              llvm::ConstantInt::get(irgen->GetIntType(), 1);


  if(!l_fa) {
    l_var = dynamic_cast<VarExpr*>(left);
    l_addr = l_var->EmitAddress();
  }

  llvm::Value *temp = new llvm::LoadInst(l_addr, "", irgen->GetBasicBlock());

  llvm::Value *ret = new llvm::LoadInst(l_addr, "", irgen->GetBasicBlock());
  llvm::Value *stor = NULL;
  char *swizzle = NULL;
  int swizzle_len = 0;



  string str = "--"; // op->toString();
  if(!str.compare("++")) {
    
      stor = (FTy) ? llvm::BinaryOperator::CreateFAdd(l, ival, "", irgen->GetBasicBlock()) :
                     llvm::BinaryOperator::CreateAdd(l, ival, "", irgen->GetBasicBlock());

  } else if(!str.compare("--")) {
  
      stor = (FTy) ? llvm::BinaryOperator::CreateFSub(l, ival, "", irgen->GetBasicBlock()) : 
                   llvm::BinaryOperator::CreateSub(l, ival, "", irgen->GetBasicBlock());
  }
  
  new llvm::StoreInst(stor, l_addr, irgen->GetBasicBlock());

  return ret;
}
//needs to be implemented
//llvm::Value* RelationalExpr::Emit() { return NULL; }

//needs to be implemented

llvm::Value* AssignExpr::Emit() {
  FieldAccess *l_fa = dynamic_cast<FieldAccess*>(left);
  char *swizzle = NULL;
  int swizzle_len = 0;
  VarExpr *l_var;

  llvm::Value *l_addr;
    l_var = dynamic_cast<VarExpr*>(left);
    l_addr = l_var->EmitAddress();


  llvm::Value *r = right->Emit();
  if(dynamic_cast<llvm::StoreInst*>(r))
    r = ((llvm::StoreInst*)r)->getValueOperand();
  llvm::Value *l;
  llvm::Value *ret = NULL;

  string str = op->toString();
  if(!str.compare("=")) {

      ret = new llvm::StoreInst(r, l_addr, irgen->GetBasicBlock());
printf("here");
  
  } else if(!str.compare("+=")) {

      l = left->Emit();
      llvm::Value *res = llvm::BinaryOperator::CreateAdd(l, r, "", irgen->GetBasicBlock());

      ret = new llvm::StoreInst(res, l_addr, irgen->GetBasicBlock());
  

  } else if(!str.compare("-=")) {

      l = left->Emit();
      llvm::Value *res = llvm::BinaryOperator::CreateSub(l, r, "", irgen->GetBasicBlock());

      ret = new llvm::StoreInst(res, l_addr, irgen->GetBasicBlock());
  }

   else if(!str.compare("*=")) {

      l = left->Emit();
      llvm::Value *res = llvm::BinaryOperator::CreateMul(l, r, "", irgen->GetBasicBlock());

      ret = new llvm::StoreInst(res, l_addr, irgen->GetBasicBlock());
  

  } else if(!str.compare("/=")) {

      l = left->Emit();
      llvm::Value *res = llvm::BinaryOperator::CreateUDiv(l, r, "", irgen->GetBasicBlock());

      ret = new llvm::StoreInst(res, l_addr, irgen->GetBasicBlock());
   
  }

  return ret;
}





//needs to be implemented
llvm::Value* LogicalExpr::Emit() { return NULL; }

//needs to be implemented
llvm::Value* EqualityExpr::Emit() {
  llvm::Value *l = left->Emit();
  llvm::Value *r = right->Emit();
  llvm::Type *ty = l->getType();

  bool FTy = (ty == (llvm::Type*)irgen->GetFloatType());
  llvm::CmpInst::OtherOps o = FTy ? llvm::CmpInst::FCmp : llvm::CmpInst::ICmp;
  llvm::CmpInst::Predicate p = llvm::CmpInst::FCMP_FALSE;

  string str = op->toString();
  if(!str.compare("=="))
    p = FTy ? llvm::CmpInst::FCMP_OEQ : llvm::ICmpInst::ICMP_EQ;

  else if(!str.compare("!="))
    p = FTy ? llvm::CmpInst::FCMP_ONE : llvm::ICmpInst::ICMP_NE;

  return llvm::CmpInst::Create(o, p, l, r, "", irgen->GetBasicBlock());
}


llvm::Value* RelationalExpr::Emit() {
  llvm::Value *l = left->Emit();
  llvm::Value *r = right->Emit();
  llvm::Type *ty = l->getType();

  bool FTy = (ty == (llvm::Type*)irgen->GetFloatType());
  llvm::CmpInst::OtherOps o = FTy ? llvm::CmpInst::FCmp : llvm::CmpInst::ICmp;
  llvm::CmpInst::Predicate p = llvm::CmpInst::FCMP_FALSE;

  string str = op->toString();
  if(!str.compare("<"))
    p = FTy ? llvm::CmpInst::FCMP_OLT : llvm::ICmpInst::ICMP_SLT;
  
  else if(!str.compare(">"))
    p = FTy ? llvm::CmpInst::FCMP_OGT : llvm::ICmpInst::ICMP_SGT;
  
  else if(!str.compare("<="))
    p = FTy ? llvm::CmpInst::FCMP_OLE : llvm::ICmpInst::ICMP_SLE;
  
  else if(!str.compare(">="))
    p = FTy ? llvm::CmpInst::FCMP_OGE : llvm::ICmpInst::ICMP_SGE;
  
  return llvm::CmpInst::Create(o, p, l, r, "", irgen->GetBasicBlock());
}

   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}

//needs to be implemented
llvm::Value* ConditionalExpr::Emit() { return NULL; }


ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
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
llvm::Value* FieldAccess::Emit() {
  char* swizzle = field->GetName();
  llvm::Value *b = base->Emit();
  const int len = strlen(swizzle);
  llvm::Value *ret = NULL;

  if(len == 1) {
    int ind;
    switch(swizzle[0]) {
      case 'x':
        ind = 0;
        break;
      case 'y':
        ind = 1;
        break;
      case 'z':
        ind = 2;
        break;
      case 'w':
        ind = 3;
        break;
      default:
        ind = 0;
	break;
    }
    llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), ind);
    ret = llvm::ExtractElementInst::Create(b, swizzle_ind, "", irgen->GetBasicBlock());

  } else {
    vector<llvm::Constant*> mask_ind;

    int i;
    for(i = 0; i < len; i++) {
      llvm::Constant *new_ind = SwizzleIndex(swizzle[i]);
      mask_ind.push_back(new_ind);
    }
    llvm::Constant *mask = llvm::ConstantVector::get(mask_ind);
    ret = new llvm::ShuffleVectorInst(b, llvm::UndefValue::get(b->getType()), mask, "", irgen->GetBasicBlock());
  }
  
  return ret;
}

llvm::Value* FieldAccess::EmitAddress() {
  llvm::Value *mem = NULL;
  VarExpr *ve = dynamic_cast<VarExpr*>(base);

  if(ve) {
    int i;
    for(i = symtab->curLevel(); i >= 0; i--) {
      mem = symtab->at(i)->getSymbol(ve->GetIdentifier()->GetName());
      if(mem) break;
    }
  }

  return mem;
}

llvm::Constant* FieldAccess::SwizzleIndex(char i) {
  int ind;
  switch (i) {
    case 'x':
      ind = 0;
      break;
    case 'y':
      ind = 1;
      break;
    case 'z':
      ind = 2;
      break;
    case 'w':
      ind = 3;
      break;
    default:
      ind = 0;
      break;
    }

    llvm::Constant* ret = llvm::ConstantInt::get(irgen->GetIntType(), ind);
    return ret;
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

