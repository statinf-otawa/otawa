/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/CodeCursor.h -- interface of CodeCursor class.
 */
#ifndef OTAWA_OSHELL_CODECURSOR_H
#define OTAWA_OSHELL_CODECURSOR_H

#include "oshell.h"

namespace otawa {

// CodeCursor class
class CodeCursor: public Cursor {
	Code *_code;
public:
	CodeCursor(Cursor *back, Code *code);
	
	// Cursor overload
	virtual void path(Output& out);
	virtual void info(Output& out);	
	virtual void list(Output& out);		
	virtual Cursor *go(CString name);
};
	
} // otawa

#endif // OTAWA_OSHELL_CODECURSOR_H
