#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "ast.hh"
#include "llvmcodegen.hh"
#include "parser.hh"

extern FILE *yyin, *ppin;
extern int yylex(), pplex();
extern char *yytext, *pptext;
extern char* pptext;

NodeStmts* final_values;

#define ARG_OPTION_L 0
#define ARG_OPTION_P 1
#define ARG_OPTION_S 2
#define ARG_OPTION_O 3
#define ARG_FAIL -1

using namespace std;

bool is_number(string s) {
    for (ulong i = 0; i < s.length(); i++) {
        if (s[i] < '0' || s[i] > '9')
            return false;
    }
    return true;
}

bool is_var_name(string s) {
    for (ulong i = 0; i < s.length(); i++) {
        if (!((s[i] <= 'z' && s[i] >= 'a') || (s[i] <= 'Z' && s[i] >= 'A')))
            return false;
    }
    return true;
}

bool is_op(string s) {
    return s == "+" || s == "-" || s == "*" || s == "/";
}

string eval(string as, string bs, string op) {
    long a = stol(as), b = stol(bs), out = 0;

    if (op == "+")
        out = a + b;
    else if (op == "-")
        out = a - b;
    else if (op == "*")
        out = a * b;
    else if (op == "/")
        out = a / b;

    return to_string(out);
}

string optimize_let(string s) {
    stack<string> st, temp_st;
    string ans = "", var, type;
    int found_let = 0, found_var = 0, brack_depth = 0, let_brack_depth = 0;

    for (ulong i = 0; i < s.length(); i++) {
        if (found_var == 1) {
            if (i < s.length() - 1)
                ans += s[i];
        } else if (s[i] == '(') {
            st.push("(");
            brack_depth++;
        } else if (s[i] == ')') {
            string val = st.top();
            st.pop();

            if (st.empty())
                continue;

            if (val != "(")
                st.pop();
            brack_depth--;

            if (brack_depth == let_brack_depth - 1 && let_brack_depth != 0) {
                ans = val;
            } else if (!is_number(st.top()))
                st.push(val);
            else {
                string as = st.top();
                st.pop();
                string op = st.top();
                st.pop();

                st.push(eval(as, val, op));
            }
        } else if (s[i] == ' ') {
            continue;
        } else {
            string temp = "";
            while (s[i] != ' ' && s[i] != ')') {
                temp += s[i];
                i++;
            }
            i--;

            if (temp == "let") {
                let_brack_depth = brack_depth;
                found_let = 1;
                continue;
            }

            if (found_let == 1) {
                var = temp;
                found_let = 2;
                continue;
            }

            if (found_let == 2) {
                type = temp;
                found_let = 0;
                continue;
            }

            if (is_op(temp))
                st.push(temp);
            else if (is_var_name(temp)) {
                while (!st.empty()) {
                    temp_st.push(st.top());
                    st.pop();
                }
                temp_st.pop();
                temp_st.pop();
                while (!temp_st.empty()) {
                    ans += temp_st.top();
                    if (is_op(temp_st.top()) || is_number(temp_st.top()))
                        ans += " ";
                    temp_st.pop();
                }
                ans += temp;
                found_var = 1;
            } else if (!is_number(st.top()))
                st.push(temp);
            else {
                string as = st.top();
                st.pop();
                string op = st.top();
                st.pop();

                st.push(eval(as, temp, op));
            }
        }
    }

    return "(let (" + var + " " + type + ") " + ans + ")";
}

string eval_if_else_condition(string s) {
    stack <string> st;
    string ans = "";
    int brack_depth = 0;

    for (ulong i = 0; i < s.length(); i++) {
        if (s[i] == '(') {
            st.push("(");
            brack_depth++;
        } else if (s[i] == ')') {
            string val = st.top();
            st.pop();

            if(st.empty())
                continue;

            if(val != "(")
                st.pop();
            brack_depth--;
            
            if (!is_number(st.top()))
                st.push(val);
            else {
                string as = st.top();
                st.pop();
                string op = st.top();
                st.pop();

                st.push(eval(as, val, op));
            }
        } else if (s[i] == ' ') {
            continue;
        } else {
            string temp = "";
            while (s[i] != ' ' && s[i] != ')') {
                temp += s[i];
                i++;
            }
            i--;
            
            if (is_op(temp))
                st.push(temp);
            else if (!is_number(st.top()))
                st.push(temp);
            else {
                string as = st.top();
                st.pop();
                string op = st.top();
                st.pop();

                st.push(eval(as, temp, op));    
            }
        }
    }

    return st.top();
}

string optimize_if_else(string s) {
    string condition = "";
    int bracket = 0;
    int index = 0;
    if (s[9] != '(') {
        for (ulong i = 9; i < s.length(); i++) {
            if (s[i] == ' ') {
                condition = s.substr(9, i - 9);
                index = i + 1;
                break;
            }
        }
    } else {
        for (ulong i = 9; i < s.length(); i++) {
            if (s[i] == '(')
                bracket++;
            if (s[i] == ')')
                bracket--;
            if (bracket == 0) {
                condition = s.substr(9, i - 9);
                for (ulong j = 0; j < condition.length(); j++)
                    if ((condition[j] <= 'z' && condition[j] >= 'a') || (condition[j] <= 'Z' && condition[j] >= 'A'))
                        return s;
                condition = eval_if_else_condition(condition);
                index = i + 2;
                break;
            }
        }
    }

    if (is_number(condition) || (condition[0] == '-' && is_number(condition.substr(1, condition.length() - 1)))) {
        int cond = stoi(condition);
        string st = "", ret = "";
        int bc = 0;
        for (ulong i = index; i < s.length(); i++) {
            if (s[i] == '(')
                bc++;
            if (s[i] == ')')
                bc--;
            if (bc == 0) {
                if (cond != 0)
                    st = s.substr(index, i - index + 1);
                else
                    st = s.substr(i + 2, s.length() - i - 3);
                string t = "";
                int brack_count = 0;
                for (ulong j = 0; j < st.length(); j++) {
                    if (st[j] == '(')
                        brack_count++;
                    if (st[j] == ')')
                        brack_count--;
                    t += st[j];
                    if (brack_count == 0) {
                        if (t.substr(0, 8) == "(if-else")
                            ret.append(optimize_if_else(t));
                        else if (t.substr(0, 4) == "(let")
                            ret.append(optimize_let(t));
                        else
                            ret.append(t);
                        t = "";
                    }
                }
                return ret;
            }
        }
    }
    return s;
}

void optimize(string s) {
    string t = "";
    FILE* fp = fopen("./bin/opt.txt", "w");
    int brack_count = 0;
    for (ulong i = 0; i < s.length(); i++) {
        if (s.substr(i, 8) == "(if-else") {
            for (ulong j = i; j < s.length(); j++) {
                if (s[j] == '(')
                    brack_count++;
                if (s[j] == ')')
                    brack_count--;
                t += s[j];
                if (brack_count == 0) {
                    fputs(optimize_if_else(t).c_str(), fp);
                    t = "";
                    i = j;
                    break;
                }
            }
        } else if (s.substr(i, 4) == "(let") {
            for (ulong j = i; j < s.length(); j++) {
                if (s[j] == '(')
                    brack_count++;
                if (s[j] == ')')
                    brack_count--;
                t += s[j];
                if (brack_count == 0) {
                    fputs(optimize_let(t).c_str(), fp);
                    t = "";
                    i = j;
                    break;
                }
            }
        } else {
            char c = s[i];
            fputc(c, fp);
        }
    }
    fclose(fp);
}

int parse_arguments(int argc, char* argv[]) {
    if (argc == 3 || argc == 4) {
        if (strlen(argv[2]) == 2 && argv[2][0] == '-') {
            if (argc == 3) {
                switch (argv[2][1]) {
                    case 'l':
                        return ARG_OPTION_L;

                    case 'p':
                        return ARG_OPTION_P;

                    case 's':
                        return ARG_OPTION_S;
                }
            } else if (argv[2][1] == 'o') {
                return ARG_OPTION_O;
            }
        }
    }

    std::cerr << "Usage:\nEach of the following options halts the compilation "
                 "process at the corresponding stage and prints the "
                 "intermediate output:\n\n";
    std::cerr << "\t`./bin/base <file_name> -l`, to tokenize the input and "
                 "print the token stream to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -p`, to parse the input and print "
                 "the abstract syntax tree (AST) to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -s`, to compile the file to LLVM "
                 "assembly and print it to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -o <output>`, to compile the file "
                 "to LLVM bitcode and write to <output>\n";
    return ARG_FAIL;
}

int main(int argc, char* argv[]) {
    int arg_option = parse_arguments(argc, argv);
    if (arg_option == ARG_FAIL) {
        exit(1);
    }

    std::string file_name(argv[1]);
    FILE* source = fopen(argv[1], "r");

    if (!source) {
        std::cerr << "File does not exists.\n";
        exit(1);
    }

    extern std::string pp_token_to_string(int token, const char* lexeme);

    // use source for lexing, and write to preprocessed.txt
    ppin = source;
    remove("preprocessed.txt");
    FILE* preprocessed = fopen("preprocessed.txt", "w+");

    if (arg_option == ARG_OPTION_L)
        printf("-- preprocessor debug --\n");

    // run preprocessor
    while (true) {
        int token = pplex();
        if (token == 1)
            fputs(pptext, preprocessed);
        if (token == 0)  // eof
            break;

        if (arg_option == ARG_OPTION_L)
            std::cout << pp_token_to_string(token, pptext) << "\n";
    }

    fclose(source);
    fseek(preprocessed, 0, SEEK_SET);  // reset preprocessed ptr

    // return 0; // for preprocessing only

    // use preprocessed for lexing
    yyin = preprocessed;

    if (arg_option == ARG_OPTION_L) {
        extern std::string lex_token_to_string(int token, const char* lexeme);
        printf("-- lexer debug --\n");

        while (true) {
            int token = yylex();
            if (token == 0) {
                break;
            }

            std::cout << lex_token_to_string(token, yytext) << "\n";
        }
        fclose(yyin);
        return 0;
    }

    final_values = nullptr;
    yyparse();

    fclose(yyin);

    if (final_values) {
        if (arg_option == ARG_OPTION_P) {
            std::cout << "(begin" << final_values->to_string() << ")" << std::endl;
            return 0;
        } 
        optimize(final_values->to_string());

        llvm::LLVMContext context;
        LLVMCompiler compiler(&context, "base");
        compiler.compile(final_values);
        if (arg_option == ARG_OPTION_S) {
            compiler.dump();
        } else {
            compiler.write(std::string(argv[3]));
        }
    } else {
        std::cerr << "empty program";
    }

    return 0;
}
