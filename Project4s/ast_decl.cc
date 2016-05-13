/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
         
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

void VarDecl::Emit() {

    
    
    llvm::Type *ty = irgen->GetType(this->type);
    
    llvm::Twine *varN = new llvm::Twine(this->id->GetName());
    
    
    if(symtab->curLevel() == 0) {
        llvm::GlobalVariable *gvar = new llvm::GlobalVariable(
                                                              *(irgen->GetOrCreateModule("")),ty,false,llvm::GlobalValue::ExternalLinkage,
                                                              llvm::Constant::getNullValue(ty),*varN,NULL);
        
        symtab->getGScope()->insertSymbol(this->id->GetName(),gvar);
    } else {
        llvm::AllocaInst *allo = new llvm::AllocaInst(ty,*varN,
                                                      irgen->GetBasicBlock());
        
        symtab->getCurScope()->insertSymbol(this->id->GetName(),allo);
    }

}

void FnDecl::Emit() {
    //symtab->curScope()->insert(this->id->GetName(), (Node*)this);
    
    llvm::Type *ty = irgen->GetType(this->returnType);


    std::vector<llvm::Type*> argT;
    for(int i=0; i < formals->NumElements(); i++) {
        Type *tempT = formals->Nth(i)->GetType();
        
        llvm::Type *parTy = irgen->GetType(tempT);
        argT.push_back(parTy);
    }
    
    llvm::ArrayRef<llvm::Type*> argR(argT);
    llvm::FunctionType *funTy = llvm::FunctionType::get(ty,argR,false);
    
    llvm::Function *funct = llvm::cast<llvm::Function>(
                                                       irgen->GetOrCreateModule("")->getOrInsertFunction(
                                                                                                         llvm::StringRef(this->id->GetName()),funTy));
    
    irgen->SetFunction(funct);
    
    llvm::Function::arg_iterator argIter = funct->arg_begin();
    for(int i = 0; i < formals->NumElements(); i++) {
        argIter->setName(formals->Nth(i)->GetId()->GetName());
        ++argIter;
    }
    
    //irgen->SetFunction(funct);
    llvm::LLVMContext *cont = irgen->GetContext();
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*cont,"entry",funct);
    irgen->SetBasicBlock(bb);
    
    symtab->insertScope();
    
    
    llvm::Function::arg_iterator locIter = funct->arg_begin();
    int i;
    for(i = 0; i < formals->NumElements(); i++) {
        llvm::Type *locT =  irgen->GetType(formals->Nth(i)->GetType());
        
        
        llvm::Twine *locN = new llvm::Twine(formals->Nth(i)->GetId()->GetName());
        
        llvm::AllocaInst *locAllo = new llvm::AllocaInst(locT,*locN,
                                                         irgen->GetBasicBlock());
        
        symtab->getCurScope()->insertSymbol(formals->Nth(i)->GetId()->GetName(),locAllo);
        new llvm::StoreInst(locIter,locAllo,irgen->GetBasicBlock());
        
        ++locIter;
    }
    
    
    if(body) {
        llvm::BasicBlock *bn = llvm::BasicBlock::Create(*cont,"next",funct);
        llvm::BranchInst::Create(bn,bb);
        irgen->SetBasicBlock(bn);
        body->Emit();
    }
    symtab->deleteScope();
    
    if(irgen->GetBasicBlock()->getTerminator() == NULL) {
        new llvm::UnreachableInst(*(irgen->GetContext()),irgen->GetBasicBlock());
    }
    
    //symtab->removeScope();
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

