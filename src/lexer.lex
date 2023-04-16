%option noyywrap

%{
#include "parser.hh"
#include <string>

extern int yyerror(std::string msg);
int lparencount = 0;
int lbracecount = 0;
int main_found = 0;
%}

%%

"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { lparencount++; return TLPAREN; }
")"       { lparencount--; 
            if(lparencount < 0) {
                yyerror("Unbalanced parenthesis");
            }
            return TRPAREN; }
"{"       { lbracecount++; return TLBRACE; }
"}"       { lbracecount--;
            if(lbracecount < 0) {
                yyerror("Unbalanced parenthesis");
            }
            return TRBRACE; }
"="       { return TEQUAL; }
"?"       { return TQUESTION; }
":"       { return TCOLON; }
","       { return TCOMMA; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
"fun"     { return TFUN; }
"ret"     { return TRET; }
"if"      { return TIF; }
"else"    { return TELSE; }
"main"    { main_found = 1; return TMAIN; }
"short"|"int"|"long" { yylval.lexeme = std::string(yytext); return TTYPE; }
[0-9]+    { yylval.lexeme = std::string(yytext); return TINT_LIT; }
[a-zA-Z]+ { yylval.lexeme = std::string(yytext); return TIDENT; }
[ \t\n]   { /* skip */ }
<<EOF>> {   if(main_found == 0) {
                yyerror("No main function found.");
            }  
            if(lparencount != 0) {
                yyerror("Unbalanced parenthesis.");
            }
            if(lbracecount != 0) {
                yyerror("Unbalanced parenthesis.");
            }
            return 0;
            }
.         { yyerror("unknown char."); }

%%

std::string lex_token_to_string(int token, const char *lexeme) {
    std::string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        case TQUESTION: s = "TQUESTION"; break;
        case TCOLON: s = "TCOLON"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;

        case TIF: s = "TIF"; break;
        case TELSE: s = "TELSE"; break;

        case TFUN: s = "TFUN"; break;
        case TLBRACE: s = "TLBRACE"; break;
        case TRBRACE: s = "TRBRACE"; break;
        case TCOMMA: s = "TCOMMA"; break;
        case TRET: s = "TRET"; break;
        
        case TTYPE: s = "TTYPE"; s.append("  ").append(lexeme); break;
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;

        case TMAIN: s = "TMAIN"; break;
    }

    return s;
}
