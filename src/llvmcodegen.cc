#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <vector>

#define MAIN_FUNC compiler->module.getFunction("main")

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

void LLVMCompiler::compile(Node *root) {
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt32Ty()},
        false
    );
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module
    );
    /* we can get this later 
        module.getFunction("printi");
    */

    // start off the codegen from AST root, probably dont need the return value
    root->llvm_codegen(this);
}

void LLVMCompiler::dump() {
    outs() << module;
}

void LLVMCompiler::write(std::string file_name) {
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
//  └―――――――――――――――――――――┘  //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler) {
    Value *last = nullptr;
    for(auto node : list) {
        last = node->llvm_codegen(compiler);
    }

    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    Function *printi_func = compiler->module.getFunction("printi");
    compiler->builder.CreateCall(printi_func, {expr});

    return expr;
}

Value *NodeInt::llvm_codegen(LLVMCompiler *compiler) {
    return compiler->builder.getInt32(value);
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler) {
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    switch(op) {
        case PLUS:
        return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
        case MINUS:
        return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
        case MULT:
        return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
        case DIV:
        return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
    }
}

Value *NodeAssign::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

Value *NodeTernary::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

Value *NodeIf::llvm_codegen(LLVMCompiler *compiler) {
    Value* cond_val = expression->llvm_codegen(compiler);
    if (!cond_val) // if the expression is null, we can't do anything
        return nullptr;
    cond_val = compiler->builder.CreateICmpNE(cond_val, compiler->builder.getInt32(0), "ifcond");
    
    Function *func = compiler->builder.GetInsertBlock()->getParent(); // get the current function we're working on
    
    BasicBlock *then_bb = BasicBlock::Create(*compiler->context, "then", func); // attach the then block to the current function
    BasicBlock *else_bb = BasicBlock::Create(*compiler->context, "else");
    BasicBlock *merge_bb = BasicBlock::Create(*compiler->context, "ifcont");
    
    compiler->builder.CreateCondBr(cond_val, then_bb, else_bb); // insert the conditional branch
    
    compiler->builder.SetInsertPoint(then_bb); //  start working on the then block
    Value *then_val = left->llvm_codegen(compiler); // let the rest of the codegen happen in the then block
    if (!then_val) 
        return nullptr;
    compiler->builder.CreateBr(merge_bb); // mandatory jump as per LLVM spec
    then_bb = compiler->builder.GetInsertBlock(); // codegen can change the current block, update then_bb for the PHI node
    
    func->getBasicBlockList().push_back(else_bb); // attach the else block to the function
    compiler->builder.SetInsertPoint(else_bb);
    Value *else_val = right->llvm_codegen(compiler);
    if (!else_val)
        return nullptr;
    compiler->builder.CreateBr(merge_bb);
    else_bb = compiler->builder.GetInsertBlock();
    
    func->getBasicBlockList().push_back(merge_bb); // attach the merge block to the function
    compiler->builder.SetInsertPoint(merge_bb); // some other part of the codegn will take care of this block
    
    PHINode *phi_node = compiler->builder.CreatePHI(compiler->builder.getInt32Ty(), 2, "iftmp");
    phi_node->addIncoming(then_val, then_bb);
    phi_node->addIncoming(else_val, else_bb);
    
    return phi_node;
}

Value *NodeReturn::llvm_codegen(LLVMCompiler *compiler) {
    Value* expr_val = expression->llvm_codegen(compiler);
    return compiler->builder.CreateRet(expr_val);
}

Value *NodeParamList::llvm_codegen(LLVMCompiler *compiler) {
    Value* last = nullptr;
    for (auto node : parameter_list) 
        last = node->llvm_codegen(compiler);

    return last;
}

Value *NodeFunctDecl::llvm_codegen(LLVMCompiler *compiler) {
    auto arg_types = std::vector<Type*>(parameter_list->parameter_list.size(), compiler->builder.getInt32Ty());
    
    FunctionType* func_type =
        FunctionType::get(compiler->builder.getInt32Ty(), arg_types, false);  // return type, arguments, varargs

    Function* func = Function::Create(func_type, GlobalValue::ExternalLinkage, name, &compiler->module);
    BasicBlock* func_entry_bb = BasicBlock::Create(*compiler->context, "entry", func);
    compiler->builder.SetInsertPoint(func_entry_bb);
    
    // start codegen for the function body
    body->llvm_codegen(compiler);

    return func;
}

Value *NodeFunctCall::llvm_codegen(LLVMCompiler* compiler) {
    Function* func = compiler->module.getFunction(name);
    auto args = std::vector<Value*>();
    for (auto node : formal_param_list)
        args.push_back(node->llvm_codegen(compiler));
    return compiler->builder.CreateCall(func, args);
}

Value *NodeDecl::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin()
    );

    AllocaInst *alloc = temp_builder.CreateAlloca(compiler->builder.getInt32Ty(), 0, identifier);

    compiler->locals[identifier] = alloc;

    return compiler->builder.CreateStore(expr, alloc);
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    return compiler->builder.CreateLoad(compiler->builder.getInt32Ty(), alloc, identifier);
}

#undef MAIN_FUNC
