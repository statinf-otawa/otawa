%{
#include <stdlib.h>
#include <otawa/util/FlowFactLoader.h>
#include "util_fft_parser.h"

elm::StringBuffer buf;
%}

%option noyywrap
%option prefix="util_fft_"
%option outfile="lex.yy.c"

DEC	[1-9][0-9]*
OCT	0[0-7]*
HEX	0[xX][0-9a-fA-F]+
BIN 0[bB][0-1]+

%x ECOM
%x TCOM
%x STR

%%

[ \t]	;
\n		;
"//"		BEGIN(ECOM);
"/*"		BEGIN(TCOM);

[;]			return *yytext;

"loop"		return LOOP;
"checksum"	return CHECKSUM;

\"			BEGIN(STR);
{DEC}		util_fft_lval._int = strtol(yytext, 0, 10); return INTEGER;
{OCT}		util_fft_lval._int = strtol(yytext, 0, 8); return INTEGER;
{HEX}		util_fft_lval._int = strtoul(yytext + 2, 0, 16); return INTEGER;
{BIN}		util_fft_lval._int = strtol(yytext + 2, 0, 2); return INTEGER;

.			return BAD_TOKEN;

<ECOM>\n	BEGIN(INITIAL);
<ECOM>.		;

<TCOM>"*/"	BEGIN(INITIAL);
<TCOM>.		;

<STR>\"		{
				util_fft_lval._str = new elm::String(buf.copyString());
				buf.reset();
				BEGIN(INITIAL);
				return STRING;
			}
<STR>\\n	buf << '\n';
<STR>\\t	buf << '\t';
<STR>\\.	buf << yytext;
<STR>.		buf << *yytext;
<STR>\n		return BAD_TOKEN;

%%
