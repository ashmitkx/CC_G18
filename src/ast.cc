#include "ast.hh"

#include <iostream>
#include <string>
#include <vector>

extern int yyerror(std::string msg);

std::string type_coercion(std::string type1, std::string type2) {
    // short, long, int
    if (type1 == "long" || type2 == "long")
        return "long";
    if (type1 == "int" || type2 == "int")
        return "int";
    return "short";
}

bool can_coerce(std::string type1, std::string type2) {
    if (type1 == "long")
        return true;
    if (type1 == "int" && type2 != "long")
        return true;
    if (type1 == "short" && type2 == "short")
        return true;
    return false;
}

NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
    dtype = type_coercion(left->dtype, right->dtype);
}

NodeIf::NodeIf(Node *expr, Node *leftptr, Node *rightptr)
{
    expression = expr;
    left = leftptr;
    right = rightptr;
}

std::string NodeIf::to_string() {
    return "(if-else " + expression->to_string() + left->to_string() + right->to_string() + ")";
}

    std::string
    NodeBinOp::to_string() {
    std::string out = "(";  
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }

    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';

    return out;
}

NodeAssign::NodeAssign(std::string id, std::string datatype, Node *expr) {
    if (!can_coerce(datatype, expr->dtype))
        yyerror("Cannot coerce " + expr->dtype + " to " + datatype + " in assignment.");

    type = ASSIGN;
    identifier = id;
    expression = expr;
}

std::string NodeAssign::to_string() {
    return "(assign " + identifier + ' ' + expression->to_string() + ")";
}

NodeTernary::NodeTernary(Node *condptr, Node *leftptr, Node *rightptr) {
    type = TERNARY;
    condition = condptr;
    left = leftptr;
    right = rightptr;
}

std::string NodeTernary::to_string() {
    return "(?: " + condition->to_string() + ' ' + left->to_string() + ' ' + right->to_string() + ')';
}

NodeInt::NodeInt(long val) {
    type = INT_LIT;
    value = val;

    if (value >= -32768 && value <= 32767)
        dtype = "short";
    else if (value >= -2147483648 && value <= 2147483647)
        dtype = "int";
    else
        dtype = "long";
}

std::string NodeInt::to_string() {
    return std::to_string(value);
}

NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeStmts::to_string() {
    std::string out = "";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    return out;
}

NodeDecl::NodeDecl(std::string id, std::string datatype, Node *expr) {
    if (!can_coerce(datatype, expr->dtype))
        yyerror("Cannot coerce " + expr->dtype + " to " + datatype + " in declaration.");

    type = ASSN;
    dtype = datatype;
    identifier = id;
    expression = expr;
}

std::string NodeDecl::to_string() {
    return "(let (" + identifier + " " + dtype +  ") " + expression->to_string() + ")";
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident, std::string datatype) {
    identifier = ident;
    dtype = datatype;
}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeFunctDecl::NodeFunctDecl(std::string id, std::string return_dtype, std::vector<std::pair<std::string, std::string>>& par_list, Node* function_body) {
    name = id;
    body = function_body;
    return_type = return_dtype;
    parameter_list = par_list;
}

std::string NodeFunctDecl:: to_string() {
    std::string parameters = "(";
    for(auto& param: parameter_list)
    {
        parameters += "(" + param.first + " " + param.second + ") ";
    }
    parameters += ')';
    
    return "(fun " + name + parameters + " : " + return_type + body -> to_string() + ")";
}

// NodeFunctCall::NodeFunctCall(std::string id, std::vector<Node*>& arg_list) {
//     name = id;
//     arguments = arg_list;
// }

// std::string NodeFunctCall::to_string() {
//     std::string args = "(";
//     for(auto& arg: arguments)
//     {
//         args += arg -> to_string() + " ";
//     }
//     args += ')';
    
//     return "(call " + name + args + ")";
// }
