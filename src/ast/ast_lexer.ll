%{
/*
 *	Lexer for Heptane AST file
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <otawa/ast.h>
#include <otawa/ast/ASTLoader.h>

#	include "ast_parser.h"

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

