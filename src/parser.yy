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

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTableStack symbol_table_stack;
int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH TQUESTION TCOLON TIF TELSE TFUN TRET TLBRACE TRBRACE TCOMMA TEOF TMAIN
%token <lexeme> TINT_LIT TIDENT TTYPE
%token INT TLET TDBG
%token TSCOL TLPAREN TRPAREN TEQUAL

%type <node> Expr Stmt 
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
                
            auto empty_vector = std::vector<std::pair<std::string, std::string>>();
            $$ = new NodeFunctDecl("main", $6, empty_vector, $9);
        }

        symbol_table_stack.destroy_context();
     }
     /* | TMAIN TLPAREN TRPAREN 
        {
            if(!symbol_table_stack.contains("main", true)) {
                // tried to call undeclared function, so error
                yyerror("tried to call undeclared function.\n");
            } else {
                $$ = new NodeFunctCall("main", std::vector<NodeExpr*>());
            }
        } */
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
     ;
     
%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}
