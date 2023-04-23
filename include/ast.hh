#ifndef AST_HH
#define AST_HH

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

struct LLVMCompiler;

/**
Base node class. Defined as `abstract`.
*/
struct Node {
    enum NodeType {
        BIN_OP, INT_LIT, STMTS, ASSN, DBG, IDENT, TERNARY, ASSIGN, PARAM_LIST, RETURN
    } type;
    
    std::string dtype;
    virtual std::string to_string() = 0;
    virtual llvm::Value *llvm_codegen(LLVMCompiler *compiler) = 0;
};

/**
    Node for list of statements
*/
struct NodeStmts : public Node {
    std::vector<Node*> list;

    NodeStmts();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for binary operations
*/
struct NodeBinOp : public Node {
    enum Op {
        PLUS, MINUS, MULT, DIV
    } op;

    Node *left, *right;

    NodeBinOp(Op op, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for ternary operations
*/
struct NodeTernary : public Node {
    Node *condition, *left, *right;

    NodeTernary(Node *condptr, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for assignment operations
*/
struct NodeAssign : public Node {
    std::string identifier;
    Node *expression;

    NodeAssign(std::string id, std::string datatype, Node* expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for integer literals
*/
struct NodeInt : public Node {
    long long value;

    NodeInt(long long val);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for variable assignments
*/
struct NodeDecl : public Node {
    std::string identifier;
    Node *expression;

    NodeDecl(std::string id, std::string datatype, Node* expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for `dbg` statements
*/
struct NodeDebug : public Node {
    Node *expression;

    NodeDebug(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for identifiers
*/
struct NodeIdent : public Node {
    std::string identifier;

    NodeIdent(std::string ident, std::string datatype);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};
struct NodeIf: public Node {
    Node *expression, *left, *right;
    NodeIf(Node *expression, Node *left, Node *right);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeParamList: public Node {
    std::vector<NodeIdent*> parameter_list;
    
    NodeParamList();
    void push_back(NodeIdent* param);
    std::string to_string();
    llvm::Value* llvm_codegen(LLVMCompiler* compiler);
};

struct NodeFunctDecl: public Node
{
    NodeFunctDecl(std::string id,
              std::string return_datatype,
              NodeParamList* param_list,
              Node* body);
    
    Node* body;
    std::string name;
    NodeParamList* parameter_list;
    std::string return_type;
    std::string to_string();
    bool has_return;
    
    llvm::Value* llvm_codegen(LLVMCompiler* compiler);
};


struct NodeReturn : public Node {
    Node *expression;
    NodeReturn(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeFunctCall: public Node {
    NodeFunctCall(std::string id, std::vector<NodeIdent*>& parameter_list, NodeFunctDecl* funct_decl); 
    
    std::string name;
    std::vector<NodeIdent*> arguments; 
    std::vector<NodeIdent*> formal_param_list;
    bool valid;
    bool check_parameters(std::vector<NodeIdent*>& actual_parameter_list, std::vector<NodeIdent*>& formal_parameter_list);
    std::string to_string();
    
    llvm::Value* llvm_codegen(LLVMCompiler* compiler);
};

#endif
