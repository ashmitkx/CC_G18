%define api.value.type { ParserValue }

%code requires {
#include <iostream>
#include <vector>
#include <string>

#include "parser_util.hh"
#include "symbol.hh"

}

%code {

#include <cstdlib>
#include <unordered_map>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTableStack symbol_table_stack
std::unordered_map<std::string, NodeFunctDecl*> function_table;
int yyerror(std::string msg);
}

%token TPLUS TDASH TSTAR TSLASH TQUESTION TCOLON TIF TELSE TFUN TRET TLBRACE TRBRACE TCOMMA TEOF TMAIN
%token <lexeme> TINT_LIT TIDENT TTYPE
%token INT TLET TDBG
%token TSCOL TLPAREN TRPAREN TEQUAL

%type <node> Expr Stmt ParamList ActualParamList
%type <stmts> Program StmtList

%right TQUESTION TCOLON
%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList 
        { final_values = $1; }
	    ;

StmtList : Stmt                
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt 
         { $$->push_back($2); }
	     ;
Stmt : TLET TIDENT TCOLON TTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table_stack.contains($2, true)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table_stack.insert($2, $4);
            
            $$ = new NodeDecl($2, symbol_table_stack.get_type($2), $6);
        }
     }
     | TDBG Expr TSCOL
     { 
        $$ = new NodeDebug($2);
     }
     | TIDENT TEQUAL Expr TSCOL
     { 
        if(symbol_table_stack.contains($1, false)) {
            $$ = new NodeAssign($1, symbol_table_stack.get_type($1), $3);
        } else {
            yyerror("tried to assign to undeclared variable.\n");
        }
     }
     | TIF Expr TLBRACE {
        symbol_table_stack.create_scope();
     } StmtList TRBRACE {
        symbol_table_stack.destroy_scope();
     } TELSE TLBRACE {
        symbol_table_stack.create_scope();
     } StmtList TRBRACE 
     {
        $$ = new NodeIf($2, $5, $11);
        symbol_table_stack.destroy_scope();
     }
     | TFUN TMAIN TLPAREN TRPAREN TCOLON TTYPE TLBRACE {
        symbol_table_stack.create_context();
     } StmtList TRBRACE
     {
        if(symbol_table_stack.contains("main", true)) {
            // tried to redeclare function, so error
            yyerror("tried to redeclare function.\n");
        } else if ($6 != "int") {
            // main function must return int
            yyerror("main function must return int.\n");
        } else {
            symbol_table_stack.insert("main", $6);  
               
            NodeParamList* empty_vector = new NodeParamList();
            $$ = new NodeFunctDecl("main", $6, empty_vector, $8);

        }

        symbol_table_stack.destroy_context();
     }
     | TFUN TIDENT TLPAREN ParamList TRPAREN TCOLON TTYPE TLBRACE StmtList TRBRACE
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare function, so error
            yyerror("tried to redeclare function.\n");
        } else {
            symbol_table.insert($2, $7);
            
            $$ = new NodeFunctDecl($2, $7, (NodeParamList*)$4, $9);
            function_table[$2] = ((NodeFunctDecl*)$$);
        }
     }
     | TFUN TIDENT TLPAREN TRPAREN TCOLON TTYPE TLBRACE StmtList TRBRACE
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare function, so error
            yyerror("tried to redeclare function.\n");
        } else {
            symbol_table.insert($2, $6);
            
            NodeParamList* empty_vector = new NodeParamList();
            $$ = new NodeFunctDecl($2, $6, empty_vector, $8);
            function_table[$2] = ((NodeFunctDecl*)$$);
        }
     }
     | TRET Expr TSCOL
     { 
        $$ = new NodeReturn($2);
     }
     ;
     
ParamList : TIDENT TCOLON TTYPE
          { $$ = new NodeParamList();
            symbol_table.insert($1, $3);
            NodeIdent* ident = new NodeIdent($1, $3);
            ((NodeParamList*)$$)->push_back(ident); }//add tident and ttypr to symbol table
          | ParamList TCOMMA TIDENT TCOLON TTYPE
          { symbol_table.insert($3, $5);
            NodeIdent* ident = new NodeIdent($3, $5);
            ((NodeParamList*)$$)->push_back(ident); }
          ;
          
ActualParamList : TIDENT
                { $$ = new NodeParamList();
                  NodeIdent* ident = new NodeIdent($1, symbol_table.get_type($1));
                  ((NodeParamList*)$$)->push_back(ident); }
                | ActualParamList TCOMMA TIDENT
                { NodeIdent* ident = new NodeIdent($3, symbol_table.get_type($3));
                  ((NodeParamList*)$$)->push_back(ident); }
                ;

Expr : TINT_LIT               
     { $$ = new NodeInt(stol($1)); }
     | TIDENT
     { 
        if(symbol_table_stack.contains($1, false))
            $$ = new NodeIdent($1, symbol_table_stack.get_type($1)); 
        else
            yyerror("using undeclared variable: " + $1 + "\n");
     }
     | Expr TQUESTION Expr TCOLON Expr
     { $$ = new NodeTernary($1, $3, $5); }
     | Expr TPLUS Expr
     { $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); }
     | Expr TDASH Expr
     { $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); }
     | Expr TSTAR Expr
     { $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); }
     | Expr TSLASH Expr
     { $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); }
     | TLPAREN Expr TRPAREN { $$ = $2; }
     | TIDENT TLPAREN TRPAREN
     {
        if(symbol_table.contains($1)) {
            std::vector<NodeIdent *> empty_vector;
            $$ = new NodeFunctCall($1, empty_vector, function_table[$1]);
        } else {
            yyerror("tried to call undeclared function.\n");
        }
     }
     | TIDENT TLPAREN ActualParamList TRPAREN
     {
        if(symbol_table.contains($1)) {
            $$ = new NodeFunctCall($1, ((NodeParamList*)$3) -> parameter_list, function_table[$1]);
        } else {
            yyerror("tried to call undeclared function.\n");
        }
     }
     ;
     
%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}
