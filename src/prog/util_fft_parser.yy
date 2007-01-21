/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 * 
 * src/prog/util_fft_parser.yy -- parser of FlowFactLoader.
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
;

%%

// Error managed
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg) {
	loader->onError(msg);
}
