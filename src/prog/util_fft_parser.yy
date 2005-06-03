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
	int _int;
}

%token <_int> INTEGER
%token LOOP
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
;

%%

// Error managed
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg) {
	loader->onError("ERROR: %s", msg);
}
