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
#include <otawa/dfa/State.h>
#include "util_fft_parser.h"

elm::StringBuffer buf;
namespace otawa {
	int fft_line = 0;
}

%}

%option noyywrap
%option prefix="util_fft_"
%option outfile="lex.yy.c"

DEC	[1-9][0-9]*
OCT	0[0-7]*
HEX	0[xX][0-9a-fA-F]+
BIN 0[bB][0-1]+
SYM [?;,+-/:@=\[\]()]

%x ECOM
%x TCOM
%x STR

%%

[ \t]	;
\n		otawa::fft_line++;
"//"		BEGIN(ECOM);
"/*"		BEGIN(TCOM);

{SYM}			return *yytext;
".."			return DOT_DOT;

"access"		return KW_ACCESS;
"call"			return KW_CALL;
"checksum"		return CHECKSUM;
"control"		return KW_CONTROL;
"entry"			return KW_ENTRY;
"ignore"		return KW_IGNORE;
"ignorecontrol"	return KW_IGNORECONTROL;
"ignoreseq"		return KW_IGNORESEQ;
"in"			return KW_IN;
"int8"			util_fft_lval.type = &otawa::Type::int8_type; return TYPE;
"int16"			util_fft_lval.type = &otawa::Type::int16_type; return TYPE;
"int32"			util_fft_lval.type = &otawa::Type::int32_type; return TYPE;
"int64"			util_fft_lval.type = &otawa::Type::int64_type; return TYPE;
"loop"			return LOOP;
"max"			return KW_MAX;
"memory"		return KW_MEMORY;
"multibranch"	return KW_MULTIBRANCH;
"multicall"		return KW_MULTICALL;
"nocall"		return KW_NOCALL;
"preserve"		return KW_PRESERVE;
"no"			return KW_NO;
"noreturn"		return NORETURN;
"return"		return RETURN;
"seq"			return KW_SEQ;
"to"			return KW_TO;
"total"			return KW_TOTAL;
"uint8"			util_fft_lval.type = &otawa::Type::uint8_type; return TYPE;
"uint16"		util_fft_lval.type = &otawa::Type::uint16_type; return TYPE;
"uint32"		util_fft_lval.type = &otawa::Type::uint32_type; return TYPE;
"uint64"		util_fft_lval.type = &otawa::Type::uint64_type; return TYPE;

\"			BEGIN(STR);
{DEC}		util_fft_lval._int = strtol(yytext, 0, 10); return INTEGER;
{OCT}		util_fft_lval._int = strtol(yytext, 0, 8); return INTEGER;
{HEX}		util_fft_lval._int = strtoul(yytext + 2, 0, 16); return INTEGER;
{BIN}		util_fft_lval._int = strtol(yytext + 2, 0, 2); return INTEGER;

.			elm::cerr << "[" << *yytext << "]\n"; return BAD_TOKEN;

<ECOM>\n	BEGIN(INITIAL); otawa::fft_line++;
<ECOM>.		;

<TCOM>"*/"	BEGIN(INITIAL);
<TCOM>\n	otawa::fft_line++;
<TCOM>.		;

<STR>\"		{
				util_fft_lval._str = new elm::String(buf.copyString());
				buf.reset();
				BEGIN(INITIAL);
				return STRING;
			}
<STR>\\n	buf << '\n'; otawa::fft_line++;
<STR>\\t	buf << '\t';
<STR>\\.	buf << yytext;
<STR>.		buf << *yytext;
<STR>\n		return BAD_TOKEN;

%%
