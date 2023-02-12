%option noyywrap

%{
#include "parser.hh"
#include <string>
#include <map>
#include <unordered_set>
using namespace std;

map <string, string> m;
string debug = "";

extern int yyerror(std::string msg);

string substitute(string ident) {
    unordered_set<string> seen;
    seen.clear();
    
    while(m.count(ident) > 0 && m[ident] != "") {
        if (seen.count(ident) > 0) {
            yyerror("recursive definition");
            return 0;
        }
        
        seen.insert(ident);
        ident = m[ident];
    }
    
    return ident;
}
%}

%%

"#def "     {
    int e;
    int state = 0; //  0: reading key, 1: reading value, 2: allowing newline 
    string key = "", value = "";
    
    while((e = yyinput()) != 0) {
        if (state == 0 && e == '\n') {
            break;
        }
        // switch to state 1 if space is found
        else if (state == 0 && e == ' ') {
            state = 1;
            continue;
        } else if (state == 1) { 
            if (e == '\\') { // allow newline
                state = 2;
                continue;
            }
            else if (e == '\n') { // end of definition
                break;
            }
        } else if (state == 2) { // read any char after '\', including newline
            state = 1;
        }
        
        if (state == 0) 
            key += e;
        else 
            value += e;
    }
    
    if (value == "")
        m[key] = "1";
    else 
        m[key] = value;
}
"#undef "   {
    int e; 
    string key = "";
    
    // read key
    while((e = yyinput()) != 0 && e != '\n') 
        key += e; 
            
    m[key] = "";
}
"//"      {
    int d; 
    while((d = yyinput()) != 0 && d != '\n'); // do nothing lmao
}
"/*"      {
    int c; 
    while((c = yyinput()) != 0) {
        if(c == '*' && (c = yyinput()) == '/') // check if next char is '/'
            break; 
        else if (c == '*') unput(c); 
    }
}
"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
[0-9]+    { yylval.lexeme = std::string(yytext); return TINT_LIT; }
[a-zA-Z]+ {
    // try to substitute if found in map
    string sub = substitute(std::string(yytext));
    
    /* 
        if substitution is done, unput the substituted string
        if substitution string is shorter than original string,
        unput spaces to fill the original string.
     */
    
    if (sub != std::string(yytext)) { // if substitution is done
        if(sub.length() <= strlen(yytext)) { 
            int len = sub.length() - 1;
            for (int i = strlen(yytext) - 1; i >= 0; i--) {
                if(i > len) 
                    unput(' ');
                else 
                    unput(sub[i]);
            }
        }
        else {
            for (int i = sub.length() - 1; i >= 0; i--) {
                unput(sub[i]);
            }
        }
    } else { 
        yylval.lexeme = sub;
        return TIDENT;
    }
}
[ \t\n]   { /* skip */ }
.         { yyerror("unknown char"); }

%%

std::string token_to_string(int token, const char *lexeme) {
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
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }
    
    /* printf("%s\n", m["NOBODY"].c_str()); */
    
    /* for (int i = 0; i < debug.length(); i++)
        printf("%d ", debug[i]); */
    
    s.append(" -- dbg: ").append(debug);
    
    return s;
}
