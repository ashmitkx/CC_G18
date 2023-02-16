%option noyywrap
%option prefix="pp"

%{
#include "parser.hh"
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;

unordered_map <string, string> m;
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

"#def"[ ]+     {
    debug = yytext;
    
    char e;
    int state = 0; //  0: reading key, 1: reading value, 2: allowing newline 
    string key = "", value = "";
    
    while((e = yyinput()) != 0) {
        if (state == 0) {
            if (e == '\n') // end of definition
                break;
            else if (e == ' ') // switch to state 1 if space is found
                state = 1;
            else
                key += e;
        } else if (state == 1) { 
            if (e == '\\') // allow newline
                state = 2;
            else if (e == '\n') // end of definition
                break;
            else if (value == "" && e == ' ') // ignore space between key and value
                continue;
            else
                value += e;
        } else if (state == 2) { // read any char after '\', including newline
            value += e;
            state = 1;
        }
    }
    
    if (value == "")
        m[key] = "1";
    else 
        m[key] = value;
}
"#undef"[ ]+    {
    char e; 
    string key = "";
    
    // read key
    while((e = yyinput()) != 0 && e != '\n') 
        key += e; 
            
    m[key] = "";
}
[a-zA-Z0-9_]+   {
    // try to substitute if found in map
    string sub = substitute(std::string(pptext));
    
    // if substitution is done, unput the substituted string
    if (sub != std::string(pptext)) // if substitution is done
        for (int i = sub.length() - 1; i >= 0; i--)
            unput(sub[i]);
    else  
        return 1;
}
"//".*                                  { /* do nothing */ }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]     { /* do nothing */ }
.|\n|\r                                 { return 1; }

%%

std::string pp_token_to_string(int token, const char *lexeme) {
    std::string s;
    s += "--" + debug + "--";
    return s;
}
