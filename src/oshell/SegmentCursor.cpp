/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/SegmentCursor.cpp -- implementation of SegmentCursor class.
 */

#include <stdlib.h>
#include "SegmentCursor.h"
#include "CodeCursor.h"

namespace otawa {

/**
 * @class SegmentCursor
 * Class for exploring segments of a program file.
 */


/**
 * Build a new segment cursor.
 * @param back		Back cursor.
 * @param segement	Segment to process.
 */	
SegmentCursor::SegmentCursor(Cursor *back, Segment *segment)
: Cursor(back), seg(segment) {
}


// Cursor overload
void SegmentCursor::path(Output& out) {
	back()->path(out);
	out << '#' << seg->name();
}


// Cursor overload
void SegmentCursor::info(Output& out) {
	out << "[Segment]\n";
	out << "name=" << seg->name() << '\n';
	out << "address=" << seg->address() << '\n';
	out << "size=" << (int)seg->size() << '\n';
	out << "flags=";
	if(seg->flags() & Segment::EXECUTABLE)
		out << " EXEC";
	if(seg->flags() & Segment::WRITABLE)
		out << " WRITE";
	out << '\n';
}


// Cursor overload
void SegmentCursor::list(Output& out) {
	out << "Items: \n";
	int i = 0;
	for(Iterator<ProgItem *> iter(seg->items()); iter; iter++, i++) {
		out << '\t' << i << ':' << iter->name() << '(';
		out << (iter->toCode() ? "code" : "data");
		out << ") [" << iter->address() << ':' << (int)iter->size() << "]\n";
	}
	if(!i)
		out << "<none>\n";
}


// Cursor overload
Cursor *SegmentCursor::go(CString name) {
	int num = atoi(&name);
	for(Iterator<ProgItem *> iter(seg->items()); iter; iter++, num--)
		if(!num) {
			Code *code = iter->toCode();
			if(!code)
				break;
			else
				return new CodeCursor(this, code);
		}
	return back()->go(name);
}

} // otawa
