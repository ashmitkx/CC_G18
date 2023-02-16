#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
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
            std::cout << final_values->to_string() << std::endl;
            return 0;
        }

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
