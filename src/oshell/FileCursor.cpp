/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FileCursor.cpp -- implementation of FileCursor class.
 */

#include <stdlib.h>
#include "FileCursor.h"
#include "SegmentCursor.h"

namespace otawa {

/**
 * @class FileCursor
 * This class is used for exploring a program file (oshell utility).
 */


/**
 * Build a new file cursor.
 * @param back	Back cursor.
 * @param file		Current file.
 */
FileCursor::FileCursor(Cursor *back, File *file): Cursor(back), _file(file) {
}


// Cursor overload
void FileCursor::path(Output& out) {
	out << _file->name();
}


// Cursor overload
void FileCursor::info(Output& out) {
	out << "[File]\n";
	out << "path=" << _file->name() << '\n';
};


// Cursor overload
void FileCursor::list(Output& out) {
	out << "Segments: \n";
	int i = 0;
	for(Iterator<Segment *> iter(_file->segments()); iter; iter++, i++) {
		out << '\t' << i << ':' << iter->name()
			 << " [" << iter->address()
			 << ':' << (int)iter->size() << ']';
		if(iter->flags() & Segment::EXECUTABLE)
			out << " EXEC";
		if(iter->flags() & Segment::WRITABLE)
			out << " WRITE";
		out << '\n';
	}
	if(!i)
		out << "<none>\n";
}


// Cursor overload
Cursor *FileCursor::go(CString name) {
	int num = atoi(&name);
	for(Iterator<Segment *> iter(_file->segments()); iter; iter++, num--)
		if(!num)
			return new SegmentCursor(this, *iter);
	return back()->go(name);
}
		
} // otawa
