/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/Process.h -- implementation for heptane::Process class.
 */

#include "Process.h"

// Lexer and parser protos
void heptane_init(io::InStream *input);
void heptane_destroy(void);
int heptane_parse(void);
extern String heptane_message;
extern otawa::Process *heptane_process;
extern otawa::File *heptane_file;

namespace otawa { namespace heptane {

	
/**
 * Build a new process.
 * @param _man	Current manager.
 * @param props	Properties.
 */
Process::Process(Manager *_man, PropList& props): gliss::Process(_man, props) {
}
	
// Process overload
::otawa::File *Process::loadFile(CString path) {
	cout.flush();
	
	// Call the parent loder method
	otawa::File *result = gliss::Process::loadFile(path);
	if(!result)
		throw LoadException("Cannot load \"%s\"", &path);
	
	// Open the AST file
	String ast_path = path + ".ast";
	io::InFileStream input(ast_path.toCString());
	if(!input.isReady()) {
		clear();
		throw LoadException("Cannot load AST file \"%s\".",
			&ast_path.toCString());
	}
		
	// Read the AST
	heptane_init(&input);
	heptane_process = this;
	heptane_file = result;
	int parse_result = heptane_parse();
	heptane_destroy();
	if(parse_result) {
		clear();
		throw LoadException(heptane_message);
	}
		
	// Return result
	cout.flush();
	return result;
}

} } // otawa::heptane
