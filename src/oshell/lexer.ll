%{
/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/lexer.ll -- lexer for OSHELL utility.
 */

#include <string.h>
#include "parser.h"
%}

%option noyywrap
%option prefix="oshell_"
%option outfile="lex.yy.c"

%x SWALLOW

BLANK	[ \t\v]*
WORD  [^ \t\v\n]+

%%

{BLANK}	;
\n			return NL;
"back"		return BACK;
"list"		return LIST;
"info"		return INFO;
"quit"		return QUIT;
"exit"		return QUIT;
"go"		return GO;
"help"		return HELP;
"dump"		return DUMP;
{WORD}		oshell_lval.str = strdup(yytext); return WORD;
.			BEGIN(SWALLOW);

<SWALLOW>\n	BEGIN(INITIAL); return ERROR;
<SWALLOW>.		;

%%
