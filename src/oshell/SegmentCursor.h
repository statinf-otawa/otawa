/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/SegmentCursor.h -- interface of SegmentCursor class.
 */
#ifndef OTAWA_OSHELL_SEGMENTCURSOR_H
#define OTAWA_OSHELL_SEGMENTCURSOR_H

#include "oshell.h"

namespace otawa {

// SegmentCursor class
class SegmentCursor: public Cursor {
	Segment *seg;
public:
	SegmentCursor(Cursor *back, Segment *segment);
	
	// Cursor overload
	virtual void path(Output& out);
	virtual void info(Output& out);	
	virtual void list(Output& out);	
	virtual Cursor *go(CString name);
};

} // otawa

#endif // OTAWA_OSHELL_SEGMENTCURSOR_H
