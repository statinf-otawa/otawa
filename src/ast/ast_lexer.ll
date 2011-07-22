%{
/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ast_lexer.ll -- lexer for Heptane AST file.
 */

#include <string.h>
#include <otawa/ast.h>

#ifdef __APPLE__
#	include "ast_parser.hpp"
#else
#	include "ast_parser.h"
#endif
%}

%option noyywrap
%option prefix="ast_"
%option outfile="lex.yy.c"
%option yylineno
%option never-interactive

BLANK	[ \t\n]
NAME	[a-zA-Z_][a-zA-Z_0-9]*
INT		[0-9]+
FRAC	\.{INT}
EXP		[eE][+-]{INT}
NUMBER	{INT}|{INT}{FRAC}|{INT}{EXP}|{INT}{FRAC}{EXP}
SEP		[,;(){}=\[\]]

%%

{BLANK}		;
{NUMBER}	return NUMBER;

[+-]		return BI_OP;
"**"		return BIN_OP;
">="		return BIN_OP;
"<="		return BIN_OP;
{SEP}		return *yytext;
[*/.<>#]	return BIN_OP;

"Vide"		return VIDE;
"Appel"		return APPEL;
"Code"		return CODE;
"Seq"		return SEQ;
"If"		return IF;
"While"		return WHILE;
"DoWhile"	return DOWHILE;
"For"		return FOR;

{NAME}:		ast_lval.str = strdup(yytext); return _LABEL;
{NAME}		ast_lval.str = strdup(yytext); return NAME;

.			return ERROR;

%%

