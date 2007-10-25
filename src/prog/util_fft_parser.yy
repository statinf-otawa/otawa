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
using namespace elm;
int util_fft_lex(void);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
%}

%name-prefix="util_fft_"
%locations
%defines
%error-verbose
%parse-param {otawa::FlowFactLoader *loader}

%union {
	long _int;
	elm::String *_str;
}

%token <_int> INTEGER
%token <_str> STRING;
%token LOOP
%token CHECKSUM
%token BAD_TOKEN
%token RETURN
%token NORETURN


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
	LOOP INTEGER INTEGER ';'
		{
			//cout << "loop " << (void *)$2 << ", " << $3 << "\n";
			loader->onLoop((otawa::address_t)$2, $3);
		}
|	CHECKSUM STRING INTEGER ';'
		{
			//cout << "checksum = " << io::hex($2) << io::endl;
			loader->onCheckSum(*$2, $3);
			delete $2;
		}
|	RETURN INTEGER ';'
		{ loader->onReturn($2); }
|	NORETURN INTEGER ';'
		{ loader->onNoReturn($2); }
|	NORETURN STRING ';'
		{
			loader->onNoReturn(*$2);
			delete $2;
		}
;

%%

// Error managed
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg) {
	loader->onError(msg);
}
