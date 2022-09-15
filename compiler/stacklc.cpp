//*******************************************************
// Purpose: main program for lang compiler
//
// Author: Philip Howard
// Email:  phil.howard@oit.edu
//
// Date: 2/20/2015
//
//*******************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>     // for include path
#include <limits.h>     // for include path
#include <unordered_map>

#include "lex.h"
#include "cAstXml.h"
#include "cSemantics.h"
#include "cSizeOffset.h"
#include "cOptimizer.h"
#include "cCodeGen.h"

#include "../version.h"

extern int yydebug;

char Include_Path[PATH_MAX] = "";

// set the include path to be the same as the directory where the EXE is.
void set_include_path()
{
    char *ptr;

    if (readlink("/proc/self/exe", Include_Path, PATH_MAX) == -1)
    {
        strcpy(Include_Path, "");
    }

    ptr = strrchr(Include_Path, '/');
    if (ptr != NULL)
    {
        ptr++;
        *ptr = 0;
    }
    strcat(Include_Path, "library/");
}

extern cDeclsList *yyast_root;

char Input_File[200] = "";
int  Do_Debug = 0;
int  Do_Ast = 0;
int  Do_Assembler = 1;
int  Do_Listing = 0;
int  Do_Type_Checks = 1;
int  Debug_Level = 0;
std::unordered_map<int,bool> Debug_Flags;

//****************************************
int ParseDebugFlag(char *arg)
{
    if (strcmp(arg, "reduce-exprs") == 0)       return cOptimizer::oFlags::reduce_exprs;
    if (strcmp(arg, "reduce-const-vars") == 0)  return cOptimizer::oFlags::reduce_const_vars;
    if (strcmp(arg, "reduce-branches") == 0)    return cOptimizer::oFlags::reduce_branches;
    return -1;
}

//****************************************
int ParseDebugLevel(char *arg)
{
    if ((arg[0] == 'O' || arg[0] == 'o') && (strlen(arg) == 2))
        return arg[1] - '0';
    return -1;
}

void Process_Args(int argc, char **argv)
{
    for (int ii=1; ii<argc; ii++)
    {
        if (argv[ii][0] == '-')
        {
            int debugLevel,flag;
            char *arg = &argv[ii][1];
            if (strcmp(arg, "ast") == 0)
                Do_Ast = 1;
            else if (strcmp(arg, "c") == 0)
                Do_Assembler = 0;
            else if (strcmp(arg, "dbg") == 0)
            {
                Do_Debug = 1;
                Do_Ast = 1;
            }
            else if (strcmp(arg, "help") == 0)
            {
                std::cout << "stacklc -c -help -no_types -version "
                            "-yydebug -dbg -ast -o1 <file>\n";
                exit(0);
            }
            else if (strcmp(arg, "list") == 0)
            {
                Do_Listing = 1;
            }
            else if (strcmp(arg, "no_types") == 0)
            {
                Do_Type_Checks = 0;
            }
                
            else if (strcmp(arg, "version") == 0)
            {
                std::cout << "stacklc " << VERSION << " " <<__DATE__ << " " 
                    << __TIME__ << std::endl;
                exit(0);
            }
            else if (strcmp(arg, "yydebug") == 0)
                yydebug = 1;
            else if ((flag = ParseDebugFlag(arg)) != -1)
            {
                Debug_Flags.emplace(flag, true);
            }
            else if ((debugLevel = ParseDebugLevel(arg)) != -1)
            {
                Debug_Level = debugLevel;
            }
            else
                std::cerr << "Unrecognized option '" << argv[ii] << "'" << std::endl;
        }
        else
        {
            // assume input file name
            strcpy(Input_File, argv[ii]);
        }
    }
}

//****************************************
int process_file(const char *filename, cDeclsList **program, int *total_errors)
{
    int result;

    // KLUDGE: should be handled within the scanner
    yylineno = 1;
    strcpy(yycurrentfile, filename);
    yyin = fopen(filename, "r");
    if (yyin == NULL)
    {
        std::cerr << "ERROR: Unable to open file " << filename << std::endl;
        exit(-1);
    }

    result = yyparse();
    while (yyast_root != NULL)
    {
        *total_errors += yynerrs;
        if (result == 0 && *total_errors == 0)
        {
            if (*program == NULL)
                *program = yyast_root;
            else
                (*program)->AddList(yyast_root);
        }
        result = yyparse();
    }

    fclose(yyin);

    if (result != 0) return result;
    if (*total_errors != 0) return 1;
    return 0;
}

//****************************************
int main(int argc, char **argv)
{
    char outfile_name[200];
    int result = 0;

    set_include_path();
    Process_Args(argc, argv);

//    std::streambuf *cout_buf = std::cout.rdbuf();

    if (strlen(Input_File) == 0)
    {
        std::cerr << "Must specify an input file." << std::endl;
        exit(-1);
    }

    symbolTableRoot = new cSymbolTable();
    symbolTableRoot->InitDefaultTable();
    cDeclsList *program = NULL;
    int total_errors = 0;

    result = process_file(Input_File, &program, &total_errors);

    if (result == 0 && total_errors == 0 && program == NULL)
    {
        semantic_error("Source produced no output ", 0);
        exit(-2);
    }

    if (result == 0 && total_errors == 0)
    {
        yynerrs = 0;

        cSemantics semantics;
        semantics.VisitAllNodes(program);

        total_errors += yynerrs;
    }

    // need to recheck total_errors because semantics checks could add errors
    if (result == 0 && total_errors == 0)
    {
        cOptimizer optimizer(Debug_Flags, Debug_Level);
        optimizer.VisitAllNodes(program);

        cSizeOffset sizer;
        sizer.VisitAllNodes(program);

        if (Do_Ast)
        {
            strcpy(outfile_name, Input_File);
            char *ptr = strrchr(outfile_name, '.');
            if (ptr != NULL) *ptr = 0;
            strcat(outfile_name, ".ast");

            cAstXml astPrinter(outfile_name);
            astPrinter.VisitAllNodes(program);
        }

        // Create output filename
        strcpy(outfile_name, Input_File);
        char *ptr = strrchr(outfile_name, '.');
        if (ptr != NULL) *ptr = 0;
        strcat(outfile_name, ".sl");

        // coder must be in its own scope so it goes out of scope
        // and gets destructed
        {
            cCodeGen coder(outfile_name);
            coder.VisitAllNodes(program);
        }

        if (Do_Assembler)
        {
            string makebin = "slasm ";
            for (int ii=strlen(argv[0]); ii >= 0; ii-- )
            {
                if (argv[0][ii] == '/')
                {
                    argv[0][ii+1] = '\0';
                    makebin = argv[0] + makebin;
                    break;
                }
            }
            if (Do_Debug) makebin += "-dbg ";
            if (Do_Listing) makebin += "-list ";

            makebin += outfile_name;

            result = system(makebin.c_str());

            if (result < 0) fatal_error("Error creating binary output");
        }

    } else {
        std::cerr << total_errors << " Errors in compile" << std::endl;
        if (result == 0) result = total_errors;
    }

    return result;
}
