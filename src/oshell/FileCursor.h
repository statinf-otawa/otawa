/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FileCursor.h -- interface of FileCursor class.
 */
#ifndef OTAWA_OSHELL_FILECURSOR_H
#define OTAWA_OSHELL_FILECURSOR_H

#include "oshell.h"

namespace otawa {

// FileCursor class
class FileCursor: public Cursor {
	File *_file;
public:
	FileCursor(Cursor *back, File *file);

	// Cursor overload
	virtual void path(Output& out);
	virtual void info(Output& out);	
	virtual void list(Output& out);
	virtual Cursor *go(CString name);
};
	
} // otawa

#endif // OTAWA_OSHELL_FILECURSOR_H
