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
//needs to be implemented
llvm::Value* ArithmeticExpr::Emit() {
    FieldAccess *f_right = dynamic_cast<FieldAccess*>(right);
    FieldAccess *f_left = NULL;
    llvm::Value *l = NULL;
    llvm::Value *r = right->Emit();
    llvm::Value *ret = NULL;
    llvm::Type *ty = r->getType();
    char *swizzleLeft = NULL;
    char *swizzleRight = NULL;
    int swizzleLeft_len = 0;
    int swizzleRight_len = 0;
    
    if(f_right != NULL) {
        swizzleRight = f_right->GetField()->GetName(); //get the name of the field access
        swizzleRight_len = strlen(swizzleRight); //get the length of it
    }
    
    //  bool FTy = (ty != (llvm::Type*)irgen->GetIntType());
    
    bool FTy;
    if( (llvm::Type*)irgen->GetIntType() != ty) {
        FTy = true;
    }
    else{
        FTy = false;
    }
    if(left != NULL) { //if its not null left, then emit it
        l = left->Emit();
        f_left = dynamic_cast<FieldAccess*>(left); //cast it to a field access
        
        if(f_left != NULL) {
            swizzleLeft = f_left->GetField()->GetName(); //if its a field access, get the name and size of it
            swizzleLeft_len = strlen(swizzleLeft); //set the length of the swizzle accordingly
        }
    }
    
    string str = op->toString();
    if(str.compare("*") == false) {
        
         if( (swizzleRight_len == 1 && swizzleLeft_len == 0) || (swizzleRight_len == 0 && swizzleLeft_len == 1) ){
            
            if(FTy == true){
                ret = llvm::BinaryOperator::CreateFMul(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret = llvm::BinaryOperator::CreateMul(l, r, "", irgen->GetBasicBlock());
            }

      }else  if( (f_left==NULL) && (f_right!=NULL)) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFMul(l, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else if((f_left!=NULL) && (f_right==NULL)) {
            int i;
            for(i = 0; i < swizzleLeft_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(l, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFMul(leftVal, r, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(l, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = l;
            
        } else {
            
            if(FTy == true){
                ret = llvm::BinaryOperator::CreateFMul(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret = llvm::BinaryOperator::CreateMul(l, r, "", irgen->GetBasicBlock());
            }
            
        }
        
    } else if(!str.compare("/")) {
       
        if ( (swizzleRight_len == 1 && swizzleLeft_len == 0) || (swizzleRight_len == 0 && swizzleLeft_len == 1) ){
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFDiv(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateUDiv(l, r, "", irgen->GetBasicBlock());
            }
            
        } else if((f_left==NULL) && (f_right!=NULL)) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFDiv(l, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else if((f_left!=NULL) && (f_right==NULL)) {
            int i;
            for(i = 0; i < swizzleLeft_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(l, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFDiv(leftVal, r, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(l, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = l;
            
        } else {
            
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFDiv(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateUDiv(l, r, "", irgen->GetBasicBlock());
            }
            
            
        }
    }
    else if(str.compare("+") == false) {
        
        if ( (swizzleRight_len == 1 && swizzleLeft_len == 0) || (swizzleRight_len == 0 && swizzleLeft_len == 1) ){
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFAdd(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateAdd(l, r, "", irgen->GetBasicBlock());
            }
            
        } else if((f_left==NULL) && (f_right!=NULL)) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFAdd(l, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else if((f_left!=NULL) && (f_right==NULL)) {
            int i;
            for(i = 0; i < swizzleLeft_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(l, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFAdd(leftVal, r, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(l, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = l;
            
        } else {
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFAdd(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateAdd(l, r, "", irgen->GetBasicBlock());
            }
            
        }
        
    } else if( str.compare("-") == false) {
       
        if ( (swizzleRight_len == 1 && swizzleLeft_len == 0) || (swizzleRight_len == 0 && swizzleLeft_len == 1) ){
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFSub(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateSub(l, r, "", irgen->GetBasicBlock());
            }
            
        } else if((f_left==NULL) && (f_right!=NULL)) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFSub(l, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else if((f_left!=NULL) && (f_right==NULL)) {
            int i;
            for(i = 0; i < swizzleLeft_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(l, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFSub(leftVal, r, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(l, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = l;
            
        } else {
            
           if(left == NULL){
	   	llvm::Value *neg;
		if(FTy == true){
         	  neg =  llvm::ConstantFP::get(irgen->GetFloatType(), -1.f);
                  ret =  llvm::BinaryOperator::CreateFMul(neg, r, "", irgen->GetBasicBlock());
       		}	
	    	else{
           	  neg =  llvm::ConstantInt::get(irgen->GetIntType(), -1);
                  ret =  llvm::BinaryOperator::CreateMul(neg, r, "", irgen->GetBasicBlock());
       	 	}
	   }
            else{ 
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFSub(l, r, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateSub(l, r, "", irgen->GetBasicBlock());
            }
          }
            
        }
        
    } else if( str.compare("--") == false) {
        
        llvm::Value *dec;
        
        if(FTy == true){
            dec =  llvm::ConstantFP::get(irgen->GetFloatType(), 1.f);
        }
        else{
            dec =  llvm::ConstantInt::get(irgen->GetIntType(), 1);
        }
        
        
        if(f_right != NULL) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFSub(dec, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else {
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFSub(r, dec, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateSub(r, dec, "", irgen->GetBasicBlock());
            }
            
            
            VarExpr* rightVar = dynamic_cast<VarExpr*>(right);
            new llvm::StoreInst(ret, rightVar->EmitAddress(), irgen->GetBasicBlock());
        }
    } else if( str.compare("++") == false) {
        
        llvm::Value *inc;
        
        if(FTy == true){
            inc =  llvm::ConstantFP::get(irgen->GetFloatType(), 1.f);
        }
        else{
            inc =  llvm::ConstantInt::get(irgen->GetIntType(), 1);
            
        }
        
        
        if(f_right != NULL) {
            int i;
            for(i = 0; i < swizzleRight_len; i++) {
                llvm::Constant *swizzle_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Value *rightHandVal = llvm::ExtractElementInst::Create(r, swizzle_ind, "", irgen->GetBasicBlock());
                llvm::Value *resVal = llvm::BinaryOperator::CreateFAdd(inc, rightHandVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(r, resVal, swizzle_ind, "", irgen->GetBasicBlock());
            }
            ret = r;
            
        } else {
            
            if(FTy == true){
                ret =  llvm::BinaryOperator::CreateFAdd(r, inc, "", irgen->GetBasicBlock());
            }
            else{
                ret =  llvm::BinaryOperator::CreateAdd(r, inc, "", irgen->GetBasicBlock());
                
            }
            
            VarExpr* rightVar = dynamic_cast<VarExpr*>(right);
            new llvm::StoreInst(ret, rightVar->EmitAddress(), irgen->GetBasicBlock());
        }
        
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

  llvm::Value *stor = NULL;
  char *swizzle = NULL;
  int swizzle_len = 0;

  if(l_fa) {
    swizzle = l_fa->GetField()->GetName();
    swizzle_len = strlen(swizzle);
    l_addr = l_fa->EmitAddress();
  } else {
    l_var = dynamic_cast<VarExpr*>(left);
    l_addr = l_var->EmitAddress();
  }


  llvm::Value *temp = new llvm::LoadInst(l_addr, "", irgen->GetBasicBlock());

  llvm::Value *ret = new llvm::LoadInst(l_addr, "", irgen->GetBasicBlock());

  string str = op->toString();
  if(!str.compare("++")) {
    if(l_fa) {
      vector<llvm::Constant*> mask_ind;
      int i;
      for(i = 0; i < swizzle_len; i++) {
        llvm::Constant *swizzle_ind = l_fa->SwizzleIndex(swizzle[i]);
	llvm::Constant *vec_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
	llvm::Value *lhs = llvm::ExtractElementInst::Create(l, vec_ind, "", irgen->GetBasicBlock());
        llvm::Value *res = llvm::BinaryOperator::CreateFAdd(lhs, ival, "", irgen->GetBasicBlock());

	llvm::InsertElementInst::Create(temp, res, swizzle_ind, "", irgen->GetBasicBlock());
	mask_ind.push_back(swizzle_ind);
      }
      llvm::Constant *mask = llvm::ConstantVector::get(mask_ind);
      stor = new llvm::ShuffleVectorInst(temp, llvm::UndefValue::get(temp->getType()), mask, "", irgen->GetBasicBlock());

    } else
      stor = (FTy) ? llvm::BinaryOperator::CreateFAdd(l, ival, "", irgen->GetBasicBlock()) :
                     llvm::BinaryOperator::CreateAdd(l, ival, "", irgen->GetBasicBlock());

  } else if(!str.compare("--")) {
    if(l_fa) {
      vector<llvm::Constant*> mask_ind;
      int i;
      for(i = 0; i < swizzle_len; i++) {
        llvm::Constant *swizzle_ind = l_fa->SwizzleIndex(swizzle[i]);
        llvm::Constant *vec_ind = llvm::ConstantInt::get(irgen->GetIntType(), i);
        llvm::Value *lhs = llvm::ExtractElementInst::Create(l, vec_ind, "", irgen->GetBasicBlock());
        llvm::Value *res = llvm::BinaryOperator::CreateFAdd(lhs, ival, "", irgen->GetBasicBlock());

        llvm::InsertElementInst::Create(temp, res, swizzle_ind, "", irgen->GetBasicBlock());
        mask_ind.push_back(swizzle_ind);
      }
      llvm::Constant *mask = llvm::ConstantVector::get(mask_ind);
      stor = new llvm::ShuffleVectorInst(temp, llvm::UndefValue::get(temp->getType()), mask, "", irgen->GetBasicBlock());

    } else 
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
    FieldAccess *f_left = dynamic_cast<FieldAccess*>(left); //dynamic cast to see if swizzle
    FieldAccess *f_right = dynamic_cast<FieldAccess*>(right); //dynamic cast to see if swizzle
    char *swizzle = NULL;
    char *swizzleRight = NULL;
    int lengthSwizzleRight = 0;
    int lengthSwizzle = 0;
    VarExpr *leftVarExpr;
    
    llvm::Value *left_valAddr;
     if(f_left != NULL) {
        left_valAddr = f_left->EmitAddress(); //emit it
        swizzle = f_left->GetField()->GetName(); //get the name of the left
        lengthSwizzle = strlen(swizzle); //get the length of the swizzlw
        
    }else { //if its not a swizzle, then cast it to a varexpr
        leftVarExpr = dynamic_cast<VarExpr*>(left);
        left_valAddr = leftVarExpr->EmitAddress();
    }
   
    if(f_right != NULL) {
       // left_valAddr = f_left->EmitAddress(); //emit it
        swizzleRight = f_right->GetField()->GetName(); //get the name of the right
	printf(swizzleRight);
        lengthSwizzleRight = strlen(swizzleRight); //get the length of the swizzle right
        
    }



 
    llvm::Value *r = right->Emit(); //call emit and store the value in r
    if(dynamic_cast<llvm::StoreInst*>(r) != NULL ) {
        r = ((llvm::StoreInst*)(r))->getValueOperand(); //if it isn't null then store the operand value in r
    }
    
    llvm::Value *ret = NULL; //create the null return object
    llvm::Value *l;
    
    string str = op->toString();
    if(str.compare("=") == false) {
        if(f_left != NULL) { //if string isn't an assign expr and f_left isn't NULL
            llvm::Value *tempVal = new llvm::LoadInst(left_valAddr, "", irgen->GetBasicBlock());
            
            if(lengthSwizzle == 1){
                llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[0]);
		if(f_right != NULL){
                	llvm::Constant *indexSwizzleRight = f_right->SwizzleIndex(swizzleRight[0]);
                        llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexSwizzleRight, "", irgen->GetBasicBlock());
                	llvm::InsertElementInst::Create(tempVal, rightVal, indexSwizzle, "", irgen->GetBasicBlock());
                	new llvm::StoreInst(tempVal, left_valAddr, "", irgen->GetBasicBlock()); //return and store the appropriate basic blocks
		}
		else{
                llvm::InsertElementInst::Create(tempVal, r, indexSwizzle, "", irgen->GetBasicBlock());
               	new llvm::StoreInst(tempVal, left_valAddr, "", irgen->GetBasicBlock()); //return and store the appropriate basic blocks
		}

            	ret = r; // =  new llvm::StoreInst(tempVal, left_valAddr, "", irgen->GetBasicBlock());
                
            }
            
            else{
                int i;
                for(i = 0; i < lengthSwizzle; i++) { //loop throught the length of the swizzle
                    llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]); //store the index value
                   
                    llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexSwizzle, "", irgen->GetBasicBlock());

		    llvm::InsertElementInst::Create(tempVal, rightVal, indexSwizzle, "", irgen->GetBasicBlock());
                }

		/* for(i = 0; i < lengthSwizzle; i++) { //loop throught the length of the swizzle
                    llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]); //store the index value
                    llvm::Constant *indexSwizzleRight = f_right->SwizzleIndex(swizzleRight[i]); //store the index value

                    llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexSwizzleRight, "", irgen->GetBasicBlock());

		    llvm::InsertElementInst::Create(tempVal, rightVal, indexSwizzle, "", irgen->GetBasicBlock());
                }
*/

                new llvm::StoreInst(tempVal, left_valAddr, "", irgen->GetBasicBlock()); //return and store the appropriate basic blocks
		ret = r;
            }
        //    ret = r;
            
        } else { //if its not a swizzle
            ret = new llvm::StoreInst(r, left_valAddr, irgen->GetBasicBlock());
        }
        
    } else if(str.compare("*=") == false) { //same logic but with "*= case "
        if(f_left != NULL) {
            llvm::Value *tempVal = new llvm::LoadInst(left_valAddr, "", irgen->GetBasicBlock());
            int i;
            for(i = 0; i < lengthSwizzle; i++) {
                llvm::Constant *indexVec = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]);
                llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
                llvm::Value *resValue = llvm::BinaryOperator::CreateFSub(leftVal, rightVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
            }
            ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());
            
        } else { //if its not a swizzle
            l = left->Emit(); //emit it
            llvm::Value *resValue = llvm::BinaryOperator::CreateMul(l, r, "", irgen->GetBasicBlock());
            
            ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());
        }
        
    } else if(str.compare("/=") == false) { // same logic as before but with "/=" case
        if(f_left != NULL) {
            llvm::Value *tempVal = new llvm::LoadInst(left_valAddr, "", irgen->GetBasicBlock());
            int i;
            for(i = 0; i < lengthSwizzle; i++) {
                llvm::Constant *indexVec = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]);
                llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
                llvm::Value *resValue = llvm::BinaryOperator::CreateFSub(leftVal, rightVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
            }
            ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());
            
        } else { //if its not a swizzle
            l = left->Emit(); //emit it
            llvm::Value *resValue = llvm::BinaryOperator::CreateFDiv(l, r, "", irgen->GetBasicBlock());
            
            ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());
        }
        
    } else if(str.compare("+=") == false) {//same logic as before,loop throught he legth of the swizzle and get appropriate basic blocks.
        if(f_left != NULL) {
            llvm::Value *tempVal = new llvm::LoadInst(left_valAddr, "", irgen->GetBasicBlock());
	
        if( (lengthSwizzle == 1 && lengthSwizzleRight == 0) ){
		l= left ->Emit();
		llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[0]);
               // llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
        	llvm::Value *resValue = llvm::BinaryOperator::CreateFAdd(l, r, "", irgen->GetBasicBlock());
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
  	 	ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());	
	}

	else if( (lengthSwizzle == 0 && lengthSwizzleRight == 1) ){
           	l = left->Emit();
        	llvm::Value *resValue = llvm::BinaryOperator::CreateFAdd(l, r, "", irgen->GetBasicBlock());
  	 	ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());	
                     
        }

	else{
            int i;
            for(i = 0; i < lengthSwizzle; i++) {
                llvm::Constant *indexVec = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]);
                llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
                llvm::Value *resValue = llvm::BinaryOperator::CreateFAdd(leftVal, rightVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
            }
            ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());
	}
            
        } else { //if its not a swizzle
            l = left->Emit(); //emit it
            llvm::Value *resValue = llvm::BinaryOperator::CreateAdd(l, r, "", irgen->GetBasicBlock());
            
            ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());
        }
        
    } else if(str.compare("-=") == false) { // "-=" case, loop through swizzle, get the index and appropriate basic blocks.
        if(f_left != NULL) {
            llvm::Value *tempVal = new llvm::LoadInst(left_valAddr, "", irgen->GetBasicBlock());

	
        if( (lengthSwizzle == 1 && lengthSwizzleRight == 0) ){
		l= left ->Emit();
		llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[0]);
               // llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
        	llvm::Value *resValue = llvm::BinaryOperator::CreateFSub(l, r, "", irgen->GetBasicBlock());
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
  	 	ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());	
	}

	else if( (lengthSwizzle == 0 && lengthSwizzleRight == 1) ){
           	l = left->Emit();
        	llvm::Value *resValue = llvm::BinaryOperator::CreateFSub(l, r, "", irgen->GetBasicBlock());
  	 	ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());	
                     
        }
	else {
            int i;
            for(i = 0; i < lengthSwizzle; i++) {
                llvm::Constant *indexVec = llvm::ConstantInt::get(irgen->GetIntType(), i);
                llvm::Constant *indexSwizzle = f_left->SwizzleIndex(swizzle[i]);
                llvm::Value *rightVal = llvm::ExtractElementInst::Create(r, indexVec, "", irgen->GetBasicBlock());
                llvm::Value *leftVal = llvm::ExtractElementInst::Create(tempVal, indexSwizzle, "", irgen->GetBasicBlock());
                llvm::Value *resValue = llvm::BinaryOperator::CreateFSub(leftVal, rightVal, "", irgen->GetBasicBlock());
                
                llvm::InsertElementInst::Create(tempVal, resValue, indexSwizzle, "", irgen->GetBasicBlock());
            }
            ret = new llvm::StoreInst(tempVal, left_valAddr, irgen->GetBasicBlock());
	}
            
        } else { //if its not a swizzle
            l = left->Emit(); //emit it
            llvm::Value *resValue = llvm::BinaryOperator::CreateSub(l, r, "", irgen->GetBasicBlock());
            
            ret = new llvm::StoreInst(resValue, left_valAddr, irgen->GetBasicBlock());
        }
        
    }
    
    return ret; //return the ret value
}



//needs to be implemented
llvm::Value* LogicalExpr::Emit() {
  llvm::Value *leftValue = left->Emit();
  llvm::Value *rightValue = right->Emit();
  llvm::Value *returnValue = NULL;


  string str = op->toString();
    
  if(str.compare("||") == false) {
      returnValue = llvm::BinaryOperator::CreateOr(leftValue, rightValue,"", irgen->GetBasicBlock());
    }

  else if(str.compare("&&") == false) {
      returnValue = llvm::BinaryOperator::CreateAnd(leftValue, rightValue,"", irgen->GetBasicBlock());

      
  }
    return returnValue;

    
}

//needs to be implemented
    llvm::Value* EqualityExpr::Emit() {
    llvm::Value *leftVar = left->Emit();   //call emit on left and right to get the values
    llvm::Value *rightVar = right->Emit();
    llvm::Type *leftValType = leftVar->getType();  //get the type for the left variable
    
    bool typeF = (leftValType == (llvm::Type*)irgen->GetFloatType()); //bool checker
    
    llvm::CmpInst::Predicate pred = llvm::CmpInst::FCMP_FALSE;
    
    llvm::CmpInst::OtherOps ops;
    
    if(typeF != NULL){ //if typeF isn't NULL
        
        ops = llvm::CmpInst::FCmp;
    }
    else{
        ops = llvm::CmpInst::ICmp;
    }
    
    
    string str = op->toString();
    
    if(str.compare("!=") == false) { //call toSTring on the op and if is not equal to "!=" and typeF isn't NULL
        
        if(typeF != NULL){
            
            pred =  llvm::CmpInst::FCMP_ONE;
        }
        else{
            pred = llvm::ICmpInst::ICMP_NE;
        }
        
    }
    
    
    else if(str.compare("==") == false){ //if str isn't equal to "==" and typeF isn't NULL
        
        if(typeF != NULL){
            
            pred =  llvm::CmpInst::FCMP_OEQ;
        }
        else{
            pred = llvm::ICmpInst::ICMP_EQ;
        }
        
    }
    //return the basic block
    return llvm::CmpInst::Create(ops, pred, leftVar, rightVar, "", irgen->GetBasicBlock());
}


llvm::Value* RelationalExpr::Emit() {
  //call emit to get the values of left and right and get the type of the left var
    llvm::Value *leftVal = left->Emit();
    llvm::Value *rightVal = right->Emit();
    llvm::Type *leftValType = leftVal->getType();
    
    bool typeF;
    
    if( leftValType != (llvm::Type*)irgen->GetFloatType() ) {
        typeF = false;     //if leftVal type isn't a float type then typeF is false, else true
    }
    else{
        typeF = true;
    }
    
    llvm::CmpInst::OtherOps ops;
    
    if (typeF == true) { //if typeF is a float type
        ops =llvm::CmpInst::FCmp;
    }
    
    else{
        ops =llvm::CmpInst::ICmp;
    }
    
    
 llvm::CmpInst::Predicate pred = llvm::CmpInst::FCMP_FALSE;
    
    string str = op->toString();
    if(!str.compare("<=")) {  // check the string to see what expression it is, and set the typeF and pred variable accordingly
        
        if (typeF == true) {
            pred = llvm::CmpInst::FCMP_OLE;
        }
        
        else{
            pred = llvm::ICmpInst::ICMP_SLE;
        }
        
    }
    
    else if(!str.compare(">=")) { //same steps as before but for ">=" case
        
        if (typeF == true) {
            pred = llvm::CmpInst::FCMP_OGE;
        }
        
        else{
            pred = llvm::ICmpInst::ICMP_SGE;
        }
        
    }
    
   else if( str.compare("<") == false) { //same steps as before but with "<" case
        
        if (typeF == true) {
            pred = llvm::CmpInst::FCMP_OLT;
        }
        
        else{
            pred = llvm::ICmpInst::ICMP_SLT;
        }
    }
    
    else if(!str.compare(">")){  //same steps as before but with ">" case
        
        if (typeF == true) {
            pred = llvm::CmpInst::FCMP_OGT;
        }
        
        else{
            pred = llvm::ICmpInst::ICMP_SGT;
        }
        
    }
    //return the appropriate basicblock
     return llvm::CmpInst::Create(ops, pred, leftVal, rightVal, "", irgen->GetBasicBlock());
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

llvm::Value* ConditionalExpr::Emit() {

  /*   
    llvm::LLVMContext *irgenContext = irgen->GetContext();
    llvm::Function *funcUse = irgen->GetFunction();
    llvm::BasicBlock *blockThen = llvm::BasicBlock::Create(*irgenContext,"blockThen",funcUse);
    llvm::BasicBlock *blockElse = llvm::BasicBlock::Create(*irgenContext,"blockElse",funcUse);
    llvm::BasicBlock *blockIf = llvm::BasicBlock::Create(*irgenContext,"blockIf",funcUse);
    llvm::BasicBlock *blockFooter = llvm::BasicBlock::Create(*irgenContext,"blockFooter",funcUse);
    llvm::BasicBlock *blockCurrent = irgen -> GetBasicBlock(); 
*/

    llvm::Value *condValue = cond->Emit();
    llvm::Value *trueValue = trueExpr->Emit();
    llvm::Value *falseValue = falseExpr->Emit();



  return llvm::SelectInst::Create(condValue, trueValue, falseValue, "", irgen->GetBasicBlock());
}


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
//needs to be implemented
llvm::Value* FieldAccess::Emit() {
    
    llvm::Value *b = base->Emit(); //call emit on base to get the value
    llvm::Value *returnVal = NULL;
    char* nameSwizzzle = field->GetName();   //get the name of the swizzle
    const int lengthSwizzle = strlen(nameSwizzzle); //get the length of the swizzle

    
    if(lengthSwizzle != 1) {    //if swizzle length is anything but 1
        vector<llvm::Constant*> indexMaskVec; //create vector
        
        int i;
        for(i = 0; i < lengthSwizzle; i++) { //loop through length of swizzle and and the swizzle constant to the vector 
            llvm::Constant *indexConst = SwizzleIndex(nameSwizzzle[i]);
            indexMaskVec.push_back(indexConst);
        }
        llvm::Constant *maskConst = llvm::ConstantVector::get(indexMaskVec);
        returnVal = new llvm::ShuffleVectorInst(b, llvm::UndefValue::get(b->getType()), maskConst, "", irgen->GetBasicBlock());
    
    } else {  //if swizzle length = 1

        int curIndex;
        switch(nameSwizzzle[0]) { //depending on the swizzle value, set the current index (xyzw) in that order starting from 0
            case 'w':
                curIndex = 3;
                break;
                
            case 'z':
                curIndex = 2;
                break;
                
            case 'y':
                curIndex = 1;
                break;
                
            case 'x':
                curIndex = 0;
                break;
                
            default:
                curIndex = 0;
                break;
        }
        llvm::Constant *indexSwizzle = llvm::ConstantInt::get(irgen->GetIntType(), curIndex); //create constant variable that stores int type and the current index
        returnVal = llvm::ExtractElementInst::Create(b, indexSwizzle, "", irgen->GetBasicBlock()); //set the value for return val to the appropriate basic block
    }
    
    return returnVal;
}
llvm::Value* FieldAccess::EmitAddress() {
    
    VarExpr *vExpr = dynamic_cast<VarExpr*>(base);
    llvm::Value *symtabCurVal = NULL;
    
    if(vExpr != NULL) {
        int i;
        for(i = symtab->curLevel(); i >= 0; i--) {
            symtabCurVal = symtab->at(i)->getSymbol(vExpr->GetIdentifier()->GetName());
            
            if(symtabCurVal != NULL){
                break;
            }
        }
    }
    
    return symtabCurVal;
}

llvm::Constant* FieldAccess::SwizzleIndex(char lookAt) {
    int curIndex;
    switch (lookAt) {
            
        case 'w':
            curIndex = 3;
            break;
            
        case 'z':
            curIndex = 2;
            break;
            
        case 'y':
            curIndex = 1;
            break;
            
        case 'x':
            curIndex = 0;
            break;
            
        default:
            curIndex = 0;
            break;
    }
    
    llvm::Constant* returnVal = llvm::ConstantInt::get(irgen->GetIntType(), curIndex);
    return returnVal;
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

