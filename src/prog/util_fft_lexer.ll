%{
#include <stdlib.h>
#include <otawa/util/FlowFactLoader.h>
#include "util_fft_parser.h"
%}

%option noyywrap
%option prefix="util_fft_"
%option outfile="lex.yy.c"

DEC	[1-9][0-9]*
OCT	0[0-7]*
HEX	0[xX][0-9a-fA-F]+
BIN 0[bB][0-1]+

%%

[ \t]	;
\n		;

[;]		return *yytext;

"loop"	return LOOP;

{DEC}	util_fft_lval._int = strtol(yytext, 0, 10); return INTEGER;
{OCT}	util_fft_lval._int = strtol(yytext, 0, 8); return INTEGER;
{HEX}	util_fft_lval._int = strtol(yytext + 2, 0, 16); return INTEGER;
{BIN}	util_fft_lval._int = strtol(yytext + 2, 0, 2); return INTEGER;

.		return BAD_TOKEN;

%%
