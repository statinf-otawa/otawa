/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/CodeCursor.cpp -- implementation of CodeCursor class.
 */

#include <stdlib.h>
#include "CodeCursor.h"
#include "InstCursor.h"

namespace otawa {

/**
 * @class CodeCursor
 * Class for exploring program code items (oshell utility).
 */


/**
 * Build a new code cursor.
 * @param back	Back cursor.
 * @param code	Code to process.
 */
CodeCursor::CodeCursor(Cursor *back, CodeItem *code): Cursor(back), _code(code) {
}


// Cursor overload
void CodeCursor::path(Output& out) {
	back()->path(out);
	out << '/' << _code->address();
}


// Cursor overload
void CodeCursor::info(Output& out) {
	out << "[Code]\n";
	out << "address=" << _code->address() << '\n';
	out << "size=" << (int)_code->size() << '\n';
}


// Cursor overload
void CodeCursor::list(Output& out) {
	out << "Instructions \n";
	Inst *inst;
	for(inst = _code->first(); !inst->atEnd(); inst = inst->next()) {
		String label = LABEL(inst);
		if(label)
			out << label << ":\n";
		out << '\t' << inst->address() << ' ';
		inst->dump(out);
		if(inst->isControl())
			out << "\tCONTROL";
		out << '\n';
	}
}


// Cursor overload
Cursor *CodeCursor::go(CString name) {
	address_t addr = (address_t)strtol(&name, 0, 16);
	for(Inst *inst = _code->first(); !inst->atEnd(); inst = inst->next())
		if(addr == inst->address())
			return new InstCursor(this, inst);
	return back()->go(name);
}
	
} // otawa
