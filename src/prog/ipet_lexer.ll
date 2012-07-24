%{
/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_lexer.ll -- lexer for IPET files.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ExpNode.h"
#include <otawa/ipet/ConstraintLoader.h>
/*#ifdef __APPLE__
#      include "ipet_parser.hpp"
#else*/
#	include <ipet_parser.h>
//#endif
extern int ipet_line;
%}

%option noyywrap
%option prefix="ipet_"
%option outfile="lex.yy.c"

DEC		[1-9][0-9]*
OCT		0[0-9]+
HEX		0[xX][a-fA-F0-9]+
BIN		0[bB][01]+
ID		[a-zA-Z_][a-zA-Z_0-9]*
IPART	[0-9]+
FPART	\.{IPART}
EPART	[eE][+-]?{IPART}
REAL	{IPART}{FPART}|{IPART}{EPART}|{IPART}{FPART}{EPART}
COM		#.*\n

%%

[ \t]+		/* ignore spaces */ ;
{COM}		ipet_line++; /* ignore comment */ ;
"//".*		ipet_line++; /* ignore comment */ ;
\n			ipet_line++; return EOL;


"bb"		return BB;
"edge"		return EDGE;

">="		return OP_GE;
"<="		return OP_LE;
[<>=]		return *yytext;
[+\-*/()]	return *yytext;

0			ipet_lval.integer = 0; return INTEGER;
{DEC}		ipet_lval.integer = strtol(yytext, 0, 10); return INTEGER;
{OCT}		ipet_lval.integer = strtol(yytext + 1, 0, 8); return INTEGER;
{HEX}		ipet_lval.integer = strtoul(yytext + 2, 0, 16); return INTEGER;
{BIN}		ipet_lval.integer = strtol(yytext + 2, 0, 2); return INTEGER;
{REAL}		ipet_lval.real = strtod(yytext, 0); return REAL;
{ID}		ipet_lval.id = strdup(yytext); return ID;

.			ECHO;

%%
