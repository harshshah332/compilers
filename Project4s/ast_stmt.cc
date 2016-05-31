/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "symtable.h"

#include "irgen.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"                                                   


Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

void Program::Emit() {
    // TODO:
    // This is just a reference for you to get started
    //
    // You can use this as a template and create Emit() function
    // for individual node to fill in the module structure and instructions.
    //
    //IRGenerator irgen;
    llvm::Module *mod = irgen->GetOrCreateModule("foo.bc");

    symtab->insertScope();
  
    int i;
    for(i = 0; i < decls->NumElements(); i++) {
      decls->Nth(i)->Emit();
    }
  
    mod->dump();

    // write the BC into standard output
    llvm::WriteBitcodeToFile(mod, llvm::outs());
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}


llvm::Value *StmtBlock::Emit() {
    int i;
    for(i = 0; i < stmts->NumElements(); i++) {
        if(!(irgen->GetBasicBlock()->getTerminator())) {
            if(strcmp("StmtBlock", stmts->Nth(i)->GetPrintNameForNode()))
                stmts->Nth(i)->Emit();
            else {
                symtab->insertScope();
                stmts->Nth(i)->Emit();
                symtab->deleteScope();
            }
        }
    }
    
    return NULL;
}



DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

llvm::Value *DeclStmt::Emit() {

  decl->Emit();

  return NULL;
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}



llvm::Value *ConditionalStmt::Emit() {
  test->Emit();
  body->Emit();

  return NULL;
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

//needs to be implemented
llvm::Value *ForStmt::Emit() {
    llvm::LLVMContext *irgenContext = irgen->GetContext();
    llvm::Function *f = irgen->GetFunction();
    
    llvm::BasicBlock *blockStep = llvm::BasicBlock::Create(*irgenContext,"blockStep",f);
    llvm::BasicBlock *blockFooter = llvm::BasicBlock::Create(*irgenContext,"footer",f);
    llvm::BasicBlock *blockHead = llvm::BasicBlock::Create(*irgenContext,"header",f);
    llvm::BasicBlock *blockBody = llvm::BasicBlock::Create(*irgenContext,"blockBody",f);
    
    init->Emit();
    
    llvm::BranchInst::Create(blockHead, irgen->GetBasicBlock());
    irgen->SetBasicBlock(blockHead);
    llvm::Value *testVal = test->Emit();
    
    llvm::BranchInst::Create(blockBody,blockFooter,testVal,blockHead);
    
    symtab->insertScope();
    irgen->SetBasicBlock(blockBody);
    blk->push_back(blockFooter);
    clk->push_back(blockStep);
    body->Emit();
    if(irgen->GetBasicBlock()->getTerminator() == NULL) {
        llvm::BranchInst::Create(blockStep,irgen->GetBasicBlock());
    }

    symtab->deleteScope();
    
    irgen->SetBasicBlock(blockStep);
    step->Emit();
    llvm::BranchInst::Create(blockHead,blockStep);
    
    int x=1;
    if(x==1){
        x=2;
    }
    
    irgen->SetBasicBlock(blockFooter);
    blk->pop_back();
    clk->pop_back();
    
    return NULL;
}


void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

//needs to be implemented
llvm::Value *WhileStmt::Emit() {
    llvm::LLVMContext *con = irgen->GetContext();
    llvm::Function *funcUse = irgen->GetFunction();
    
    llvm::BasicBlock *blockHead = llvm::BasicBlock::Create(*con,"header",funcUse);
    llvm::BasicBlock *blockBody = llvm::BasicBlock::Create(*con,"blockBody",funcUse);
    llvm::BasicBlock *blockFooter = llvm::BasicBlock::Create(*con,"footer",funcUse);
    
    
    llvm::BranchInst::Create(blockHead, irgen->GetBasicBlock());
    irgen->SetBasicBlock(blockHead);
    llvm::Value *testVal = test->Emit();
    
    llvm::BranchInst::Create(blockBody,blockFooter,testVal,blockHead);
    
    symtab->insertScope();
    blk->push_back(blockFooter);
    clk->push_back(blockHead);
    irgen->SetBasicBlock(blockBody);
    body->Emit();
    if(blockBody->getTerminator() == NULL) {
        llvm::BranchInst::Create(blockHead,blockBody);
    }
    symtab->deleteScope();
    
    irgen->SetBasicBlock(blockFooter);
    blk->pop_back();
    clk->pop_back();
    
    return NULL;
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

//needs to be implemented
llvm::Value *IfStmt::Emit() {
  
    
    llvm::Value *testVal = test->Emit();
    
    llvm::LLVMContext *irgenContext = irgen->GetContext();
    llvm::Function *funcUse = irgen->GetFunction();
    llvm::BasicBlock *blockThen = llvm::BasicBlock::Create(*irgenContext,"blockThen",funcUse);
    llvm::BasicBlock *blockElse = NULL;
    if(elseBody){
        blockElse = llvm::BasicBlock::Create(*irgenContext,"blockElse",funcUse);
    }
    
    llvm::BasicBlock *blockFooter = llvm::BasicBlock::Create(*irgenContext,"footer",funcUse);
    
    llvm::BasicBlock *blockCurrent = irgen->GetBasicBlock();
    
    llvm::BranchInst::Create(blockThen,elseBody?blockElse:blockFooter,testVal,blockCurrent);
    
    symtab->insertScope();
    irgen->SetBasicBlock(blockThen);
    body->Emit();
    if(irgen->GetBasicBlock()->getTerminator() == NULL) { //check if its null
        llvm::BranchInst::Create(blockFooter,irgen->GetBasicBlock());
    }
    symtab->deleteScope();
    
    irgen->SetBasicBlock(blockFooter);
    
    if(elseBody != NULL) {
        symtab->insertScope();
        irgen->SetBasicBlock(blockElse);
        elseBody->Emit();
        if(irgen->GetBasicBlock()->getTerminator() == NULL) {
            llvm::BranchInst::Create(blockFooter,irgen->GetBasicBlock());
        }
        symtab->deleteScope();
        irgen->SetBasicBlock(blockFooter);
    }
    
    return NULL;
}


llvm::Value *BreakStmt::Emit() {
  llvm::BranchInst::Create(irgen->blk.top(),irgen->GetBasicBlock());  

  return NULL;
}

llvm::Value *ContinueStmt::Emit() {
  llvm::BranchInst::Create(irgen->clk.top(),irgen->GetBasicBlock());

  return NULL;
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
}

llvm::Value *ReturnStmt::Emit() {
    if(expr == NULL) {

        llvm::ReturnInst::Create(*(irgen->GetContext()),irgen->GetBasicBlock());

    } else {
    llvm::Value *retV = expr->Emit();
    llvm::ReturnInst::Create(*(irgen->GetContext()),retV,irgen->GetBasicBlock());    }
    
    return NULL;
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

llvm::Value *SwitchLabel::Emit() {
 
  stmt->Emit();

  return NULL;
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

llvm::Value *SwitchStmt::Emit() {
    
    llvm::Function *funcUse = irgen->GetFunction();
    llvm::LLVMContext *irgenContext = irgen->GetContext();
    
    int numCases = cases->NumElements(); // the number of total cases
    
    
    // the block for the footer
    llvm::BasicBlock *blockFooter = llvm::BasicBlock::Create(*irgenContext,"footer",funcUse);

    
    // the block for the default case, currrently empty
    llvm::BasicBlock *blockDefault = llvm::BasicBlock::Create(*irgenContext,"defaultBB",funcUse);
    
    
    
    blk->push_back(blockFooter);
    
    // here we create the instance of the switch
    llvm::SwitchInst *switchInstance = llvm::SwitchInst::Create(expr->Emit(),blockDefault,numCases,
                                                     irgen->GetBasicBlock());
    
    
    for(int i = 0; i < numCases; i++) {
        if(dynamic_cast<Case*>(cases->Nth(i)) != NULL ) {
            llvm::BasicBlock *caseBB = llvm::BasicBlock::Create(*irgenContext,"caseBB",funcUse);
            llvm::Value* testVal = dynamic_cast<Case*>(cases->Nth(i))->caseLabel()->Emit();
            switchInstance->addCase((llvm::ConstantInt*)testVal,caseBB);
            if(irgen->GetBasicBlock()->getTerminator() == NULL)
                llvm::BranchInst::Create(caseBB,irgen->GetBasicBlock());
            irgen->SetBasicBlock(caseBB);
            ((SwitchLabel*)cases->Nth(i))->Emit();
            
        } else if(dynamic_cast<Default*>(cases->Nth(i)) != NULL) {
            if(irgen->GetBasicBlock()->getTerminator() == NULL)
                llvm::BranchInst::Create(blockDefault,irgen->GetBasicBlock());
            irgen->SetBasicBlock(blockDefault);
            ((SwitchLabel*)cases->Nth(i))->Emit();
            
        } else {
            cases->Nth(i)->Emit();
        }
    }
    
    if(blockDefault->getTerminator() == NULL)
        llvm::BranchInst::Create(blockFooter,blockDefault);
    
    blk->pop_back();
    
    irgen->SetBasicBlock(blockFooter);
    
    return NULL;
}
