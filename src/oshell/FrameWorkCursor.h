/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FrameWorkCursor.h -- interface of FrameWorkCursor class.
 */
#ifndef OTAWA_OSHELL_FRAMEWORKCURSOR_H
#define OTAWA_OSHELL_FRAMEWORKCURSOR_H

#include "oshell.h"

namespace otawa {

// FrameWorkCursor class
class FrameWorkCursor: public Cursor {
	FrameWork *fw;
public:
	inline FrameWorkCursor(FrameWork *_fw): fw(_fw) { };
	void load(Output& out, CString path);
	void cfg(Output& out);	

	// Cursor overload
	virtual void path(Output& out);
	virtual void info(Output& out);
	virtual void list(Output& out);
	virtual Cursor *go(CString name);
	virtual AutoPtr<Cursor> back(void);
	virtual void perform(Output& out, int argc, CString argv[]);
	virtual void help(Output& out);
};
	
} // otawa

#endif // OTAWA_OSHELL_FRAMEWORKCURSOR_H
