/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/InstCursor.cpp -- implementation of InstCursor class.
 */

#include "InstCursor.h"

namespace otawa {

	
/**
 * @class InstCursor
 * Class for exploring instructions (oshell utility).
 */

	
/**
 * Build a new oinstruction cursor.
 * @param back	Back cursor.
 * @param inst		Instruction to process.
 */	
InstCursor::InstCursor(Cursor *back, Inst *inst): Cursor(back), _inst(inst) {
}


// Cursor overload
void InstCursor::path(Output& out) {
	back()->path(out);
	out << '/' << _inst->address();
}


// Cursor overload
void InstCursor::info(Output& out) {
	out << "[INSTRUCTION]\n"
		 << "\taddress = " << _inst->address() << '\n';
	out << "\tasm = ";
	_inst->dump(out);
	out << '\n';
}	

} // otawa
