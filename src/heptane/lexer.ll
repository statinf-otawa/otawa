%{
/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/lexer.ll -- lexer for Heptane AST file.
 */

#include <string.h>
#include <elm/io.h>
#include <otawa/ast.h>
using namespace elm;
using namespace otawa;
#include "parser.h"

// Input management
static io::InStream *heptane_input = 0;
int heptane_read(char *buf, int size);
#define YY_INPUT(buf, result, max) result = heptane_read(buf, max)
int heptane_line;

%}

%option noyywrap
%option prefix="heptane_"
%option outfile="lex.yy.c"

BLANK	[ \t]
NAME	[a-zA-Z_][a-zA-Z_0-9]*
INT		[0-9]+
FRAC	\.{INT}
EXP		[eE][+-]{INT}
NUMBER	{INT}|{INT}{FRAC}|{INT}{EXP}|{INT}{FRAC}{EXP}
SEP		[,;(){}=\[\]]

%%

"\n"		heptane_line++;
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

{NAME}:		heptane_lval.str = strdup(yytext); return LABEL;
{NAME}		heptane_lval.str = strdup(yytext); return NAME;

.			return ERROR;

%%


/**
 * Read some character for feeding the lexer.
 * @param buffer	Buffer to fill.
 * @param size	Buffer size.
 * @return		YY_NULL for end-of-file or the filled size.
 */
int heptane_read(char *buffer, int size) {
	int result;
	assert(heptane_input && buffer);
	result = heptane_input->read(buffer, size);
	if(result > 0)
		return result;
	else
		return YY_NULL;
}


/**
 * Initialize the lexer with the given input stream.
 * @param input	Input stream to use.
 */
void heptane_init(io::InStream *input) {
	assert(!heptane_input && input);
	heptane_input = input;
	heptane_line = 1;
}


/**
 * Destroy the current lexer.
 */
void heptane_destroy(void) {
	assert(heptane_input);
	heptane_input = 0;
}
