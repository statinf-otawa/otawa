/*
 *	$Id$
 *	F4 parser
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-10, IRIT UPS.
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
%{
#include <stdio.h>
#include <otawa/util/FlowFactLoader.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/dfa/State.h>
using namespace elm::genstruct;
int util_fft_lex(void);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
static Vector<otawa::Address> addresses;
static otawa::ContextualPath path;

// Loop counting
static int loop_max = -1, loop_total = -1;
static void reset_counts(void) {
	loop_max = -1;
	loop_total = -1;
}
%}

%name-prefix="util_fft_"
%locations
%defines
%error-verbose
%parse-param {otawa::FlowFactLoader *loader}

%union {
	int _int;
	elm::String *_str;
	otawa::Address *addr;
	const otawa::Type *type;
	otawa::dfa::Value *value;
}

%token <_int> INTEGER
%token <_str> STRING;
%token BAD_TOKEN
%token CHECKSUM
%token DOT_DOT
%token KW_ACCESS
%token KW_CALL
%token KW_CONTROL
%token KW_ENTRY
%token KW_IGNORE
%token KW_IGNORECONTROL
%token KW_IGNORESEQ
%token KW_IN
%token KW_MAX
%token KW_MEMORY
%token KW_MULTIBRANCH
%token KW_MULTICALL
%token KW_NO
%token KW_NOCALL
%token KW_PRESERVE
%token KW_REG
%token KW_SEQ
%token KW_TO
%token KW_TOTAL
%token LOOP
%token NORETURN
%token RETURN
%token TYPE
%type<addr> full_address id_or_address
%type<type> TYPE
%type<value> value

%%

file:
	/* empty */
|	commands
;

commands:
	command
|	commands command
;

command:
	LOOP full_address INTEGER opt_in ';'
		{ loader->onLoop(*$2, $3, -1, path); delete $2; path.clear(); }
|	LOOP full_address counts opt_in ';'
		{
			loader->onLoop(*$2, loop_max, loop_total, path);
			delete $2;
			path.clear();
			reset_counts();
		}
|	LOOP full_address '?' opt_in ';'
		{ loader->onUnknownLoop(*$2); delete $2; path.clear(); }
|	CHECKSUM STRING INTEGER ';'
		{
			//cout << "checksum = " << io::hex($2) << io::endl;
			loader->onCheckSum(*$2, $3);
			delete $2;
		}
|	RETURN full_address ';'
		{ loader->onReturn(*$2); delete $2; }
|	NORETURN full_address ';'
		{ loader->onNoReturn(*$2); delete $2; }
|	KW_NO RETURN full_address ';'
		{ loader->onNoReturn(*$3); delete $3; }
|	KW_NOCALL id_or_address ';'
		{ loader->onNoCall(*$2); delete $2; }
|	KW_NO KW_CALL id_or_address ';'
		{ loader->onNoCall(*$3); delete $3; }
|	KW_IGNORECONTROL full_address ';'
		{ loader->onIgnoreControl(*$2); delete $2; }
|	KW_IGNORE KW_CONTROL full_address ';'
		{ loader->onIgnoreControl(*$3); delete $3; }
|	KW_IGNORESEQ full_address ';'
		{ loader->onIgnoreSeq(*$2); delete $2; }
|	KW_IGNORE KW_SEQ full_address ';'
		{ loader->onIgnoreSeq(*$3); delete $3; }
|	KW_MULTIBRANCH { addresses.setLength(0); } multibranch
		{ }
|	KW_MULTICALL { addresses.setLength(0); } multicall
		{ }
|	KW_PRESERVE full_address ';'
		{ loader->onPreserve(*$2); delete $2; }
|	KW_IGNORE KW_ENTRY STRING ';'
		{ loader->onIgnoreEntry(*$3); delete $3; }
|	KW_MEMORY KW_ACCESS full_address full_address DOT_DOT full_address opt_in ';'
		{
			loader->onMemoryAccess(*$3, *$4, *$6, path);
			path.clear();
			delete $3;
			delete $4;
			delete $6;
		}
|	KW_REG STRING '=' value ';'
		{ loader->onRegSet(*$2, *$4); delete $2; delete $4; }
|	KW_MEMORY TYPE full_address '=' value ';'
		{ loader->onMemSet(*$3, $2, *$5); delete $3; delete $5; }
;

counts:
	count			{ }
|	count counts	{ }
;

count:
	KW_MAX INTEGER
		{
			if(loop_max >= 0)
				loader->onError(elm::_ << "several 'max' keywords");
			else
				loop_max = $2;
		}
|	KW_TOTAL INTEGER
		{
			if(loop_total >= 0)
				loader->onError(elm::_ << "several 'total' keywords");
			else
				loop_total = $2;
		}
;

multibranch:
	full_address KW_TO address_list ';'
		{ loader->onMultiBranch(*$1, addresses); delete $1; }
|	full_address KW_TO '?' ';'
		{ loader->onUnknownMultiBranch(*$1); delete $1; }
;

multicall:
	full_address KW_TO address_list ';'
		{ loader->onMultiCall(*$1, addresses); delete $1; }
|	full_address KW_TO '?' ';'
		{ loader->onUnknownMultiCall(*$1); delete $1; }
;

address_list:
	full_address
		{ addresses.add(*$1); delete $1; }
|	address_list ',' full_address
		{ addresses.add(*$3); delete $3; }
;

id_or_address:
	INTEGER
		{ $$ = new otawa::Address($1); }
|	STRING
		{ $$ = new otawa::Address(loader->addressOf(*$1)); }
;


full_address:
	INTEGER
		{ $$ = new otawa::Address($1); }
|	STRING
		{ $$ = new otawa::Address(loader->addressOf(*$1)); delete $1; }
|	STRING '+' INTEGER
		{ $$ = new otawa::Address(loader->addressOf(*$1) + $3); delete $1; }
|	STRING '-' INTEGER
		{ $$ = new otawa::Address(loader->addressOf(*$1) - $3); delete $1; }
|	STRING ':' INTEGER
		{ $$ = new otawa::Address(loader->addressOf(*$1, $3)); delete $1; }
;

opt_in:
	/* empty */
		{ }
|	KW_IN path
		{ }
;

step:	full_address
			{ path.push(otawa::ContextualStep::FUNCTION, *$1); delete $1; }
|		'@' full_address
			{ path.push(otawa::ContextualStep::CALL, *$2); delete $2; }
;

path:
	step
		{  }
|	step '/' path
		{ }
;

value:
	full_address
		{ $$ = new otawa::dfa::Value($1->offset()); delete $1; }
|	'[' full_address DOT_DOT full_address ']'
		{ $$ = new otawa::dfa::Value($2->offset(), $4->offset()); delete $2; delete $4; }
|	'(' full_address ',' INTEGER ',' INTEGER ')'
		{ $$ = new otawa::dfa::Value($2->offset(), $4, $6); delete $2; }
;

%%

// Error managed
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg) {
	loader->onError(msg);
}
