/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/InstCursor.h -- interface of InstCursor class.
 */
#ifndef OTAWA_OSHELL_INSTCURSOR_H
#define OTAWA_OSHELL_INSTCURSOR_H

#include "oshell.h"

namespace otawa {

// InstCursor class
class InstCursor: public Cursor {
	Inst *_inst;
public:
	InstCursor(Cursor *back, Inst *inst);
	
	// Cursor overload
	virtual void path(Output& out);
	virtual void info(Output& out);	
};
	
} // otawa

#endif // OTAWA_INSTRUCTION_H
