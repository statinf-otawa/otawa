/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	oshell.yy -- parser for the OShell application.
 */
%{
#include <stdio.h>
#include <stdlib.h>
#include <elm/string.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include "oshell.h"
using namespace elm;
using namespace otawa;

// Prototypes
int yylex(void);

// Globals
Manager *manager;
AutoPtr<Cursor> cursor;
int argc = 0;
CString argv[16];
OutFileStream *out_stream = 0;
Output *out = &cout;

// Display cursor
void display_cursor(void) {
	cursor->path(cout);
	cout << "> ";
	cout.flush();
}

// Handle error
void yyerror(const char *msg) {
	cout << "ERROR: " << msg << '\n';
	display_cursor();
}

#define size_t unsigned long

%}

%name-prefix="oshell_"
%error-verbose
%union {
	const char *str;
}

%token ERROR
%token <str> IDENT WORD
%token INFO
%token LIST
%token BACK
%token QUIT
%token GO
%token HELP
%token NL
%token DUMP

%type <str> word

%%

shell:
	/* empty */
|	lines
;

lines:
	line
|	line lines
;

line:
	command NL
		{ display_cursor(); }

command:
	/* Empty command */
|	LIST
		{ cursor->list(*out); }
|	INFO
		{ cursor->info(*out); }
|	BACK
		{
			try {
				cursor = cursor->back();
			}
			catch(BackException exn) {
				cerr << "ERROR: " << exn.message() << '\n';
			}
		}
|	QUIT
		{ YYACCEPT; }
|	GO word
		{
			try {
				cursor = cursor->go($2);
			}
			catch(GoException exn) {
				cerr << "ERROR: " << exn.message() << '\n';
			}
			catch(BackException exn) {
				cerr << "ERROR: no back context available !\n";
			}
		}
|	dump_header command
		{
			delete out;
			delete out_stream;
			out = &cout;
		}
|	call
		{
			try {
				cursor->perform(*out, argc, argv);
			}
			catch(PerformException exn) {
				cerr << "ERROR: " << exn.message() << '\n';
			}
			for(int i = 0; i < argc; i++) {
				free((void *)argv[i].chars());
				argv[i] = "";
			}
			argc = 0;
		}
| HELP
		{
			cerr << "[Common Commands]\n"
				"info: information about the current item.\n"
				"list: list sub-items of the current item.\n"
				"back: go back in structure tree.\n"
				"go <sub-item>: go to the given sub-item (usually index from a list).\n"
				"help: display this help (NOTE: some commands are local to an item).\n"
				"dump <file> <command>: dump the result of the command in the given file.\n"
				"quit|exit: leave Otawa Shell.\n"
				"\n[Local Commands]\n";	
			cursor->help(cerr);
		}
;

call:
	arg
|	arg call
;
arg:
	word
		{ argv[argc++] = $1; }
;

word:
	WORD
		{ $$ = $1; }
;			

dump_header:
		DUMP WORD
			{
				out_stream = new OutFileStream($2);
				if(!out_stream->isReady()) {
					delete out_stream;
					cerr << "ERROR: Cannot open the file \"" << $2 << "\".\n";
					YYABORT;
				}
				else
					out = new Output(*out_stream);
				free((void *)$2);
			}

%%


/**
 * Entry function for oshell program.
 */
int main(void) {

	// Build the framework
	manager = new Manager();
	PropList args;
	FrameWork *fw = new FrameWork(
		Loader::LOADER_Gliss_PowerPC.create(manager, args));
	
	// Start the interpreter
	cursor = Cursor::get(fw);
	display_cursor();
	while (yyparse());
	
	// Clean up
	cursor = 0;
	delete manager;
	return 0;
}
