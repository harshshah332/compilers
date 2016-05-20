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
    
    
    llvm::Type *typeCur = irgen->GetType(this->type);
    
    llvm::Twine *nameVar = new llvm::Twine(this->id->GetName());
    
    
    if(symtab->curLevel() != 0) {

        llvm::AllocaInst *instAlloc = new llvm::AllocaInst(typeCur,*nameVar, irgen->GetBasicBlock());
        symtab->getCurScope()->insertSymbol(this->id->GetName(),instAlloc);
    
    
    } else {
        
        llvm::GlobalVariable *varGlob = new llvm::GlobalVariable(*(irgen->GetOrCreateModule("")),typeCur,false,llvm::GlobalValue::ExternalLinkage, llvm::Constant::getNullValue(typeCur),*nameVar,NULL);
        
        symtab->getGScope()->insertSymbol(this->id->GetName(),varGlob);
    }
    
}

void FnDecl::Emit() {
    
    //here we start the emit funciton
    llvm::Type *typeCur = irgen->GetType(this->returnType);
    
    
    std::vector<llvm::Type*> typeArgs;
    for(int i=0; i < formals->NumElements(); i++) {
        Type *typeTemp = formals->Nth(i)->GetType();
        
        llvm::Type *parTy = irgen->GetType(typeTemp);
        typeArgs.push_back(parTy);
    }
    
    llvm::ArrayRef<llvm::Type*> refArray(typeArgs);
    llvm::FunctionType *funTy = llvm::FunctionType::get(typeCur,refArray,false);
    
    llvm::Function *funcTmp = llvm::cast<llvm::Function>( irgen->GetOrCreateModule("")->getOrInsertFunction( llvm::StringRef(this->id->GetName()),funTy));
    
    irgen->SetFunction(funcTmp);
    
    llvm::Function::arg_iterator iteratorArg = funcTmp->arg_begin();
    for(int i = 0; i < formals->NumElements(); i++) {
        iteratorArg->setName(formals->Nth(i)->GetId()->GetName());
        ++iteratorArg;
    }
    
    //get the current irgen conext
    llvm::LLVMContext *irgenContext = irgen->GetContext();
    llvm::BasicBlock *blockBasic = llvm::BasicBlock::Create(*irgenContext,"entry",funcTmp);
    irgen->SetBasicBlock(blockBasic);
    
    symtab->insertScope();
    
    
    llvm::Function::arg_iterator iteratLocation = funcTmp->arg_begin();
    int i;
    for(i = 0; i < formals->NumElements(); i++) {
        llvm::Type *typeLoc =  irgen->GetType(formals->Nth(i)->GetType());
        
        
        llvm::Twine *twineNam = new llvm::Twine(formals->Nth(i)->GetId()->GetName());
        
        llvm::AllocaInst *allocLoc = new llvm::AllocaInst(typeLoc,*twineNam,
                                                         irgen->GetBasicBlock());
        
        symtab->getCurScope()->insertSymbol(formals->Nth(i)->GetId()->GetName(),allocLoc);
        new llvm::StoreInst(iteratLocation,allocLoc,irgen->GetBasicBlock());
        
        ++iteratLocation;
    }
    
    
    if(body != NULL) {
        llvm::BasicBlock *blockBasicNext = llvm::BasicBlock::Create(*irgenContext,"next",funcTmp);
        llvm::BranchInst::Create(blockBasicNext,blockBasic);
        irgen->SetBasicBlock(blockBasicNext);
        body->Emit();
    }
    
    //delete the current scope from symtab
    symtab->deleteScope();
    
    if(irgen->GetBasicBlock()->getTerminator() == NULL) {
        new llvm::UnreachableInst(*(irgen->GetContext()),irgen->GetBasicBlock());
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

