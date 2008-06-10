/*
 *	$Id$
 *	F4 parser
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
%{
#include <otawa/util/FlowFactLoader.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <otawa/flowfact/ContextualLoopBound.h>
using namespace elm;
using namespace elm::genstruct;
int util_fft_lex(void);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
static Vector<otawa::Address> addresses;
static otawa::ContextPath<otawa::Address> path;

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
}

%token <_int> INTEGER
%token <_str> STRING;
%token LOOP
%token CHECKSUM
%token BAD_TOKEN
%token RETURN
%token NORETURN
%token KW_NOCALL
%token KW_MULTIBRANCH
%token KW_IGNORECONTROL
%token KW_TO
%token KW_PRESERVE
%token KW_IN
%token KW_MAX
%token KW_TOTAL
%type<addr> full_address id_or_address

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
|	KW_NOCALL id_or_address ';'
		{ loader->onNoCall(*$2); delete $2; }
|	KW_IGNORECONTROL full_address ';'
		{ loader->onIgnoreControl(*$2); delete $2; }
|	KW_MULTIBRANCH { addresses.setLength(0); } multibranch
		{ }
|	KW_PRESERVE full_address ';'
		{ loader->onPreserve(*$2); delete $2; }
;

counts:
	count			{ }
|	count counts	{ }
;

count:
	KW_MAX INTEGER
		{
			if(loop_max >= 0)
				loader->onError(_ << "several 'max' keywords");
			else
				loop_max = $2;
		}
|	KW_TOTAL INTEGER
		{
			if(loop_total >= 0)
				loader->onError(_ << "several 'total' keywords");
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

path:
	full_address
		{ path.push(*$1); delete $1; }
|	full_address '/' path
		{ path.push(*$1); delete $1; }
;

%%

// Error managed
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg) {
	loader->onError(msg);
}
