%{
/*
 *	$Id$
 *	F4 lexer
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
"return"	return RETURN;
"noreturn"	return NORETURN;

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
