/*
 *	$Id$
 *	Trace class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_UTIL_TRACE_H
#define OTAWA_UTIL_TRACE_H

#include <elm/string.h>
#include <elm/io/OutStream.h>
#include <elm/io/Output.h>
#include <elm/io.h>

namespace otawa {

class Level;

extern const Level L1;
extern const Level L2;
extern const Level L3;
extern const Level L4;
extern const Level L5;
extern const Level L6;
extern const Level L7;
extern const Level L8;
extern const Level L9;


class Level {
public:
	inline Level(int l)
	: _level(l) { }
	int _level;
};

class NullStream: public elm::io::OutStream {
 virtual int write (const char *buffer, int size) { return 0;}
 int flush (void) { return 0; }
 };

extern NullStream null_stream;

class Trace : public elm::io::Output {
	int _level;
	elm::String _module_name;
	elm::io::OutStream& _stream;

public:
	Trace (elm::io::OutStream& stream, int level, elm::String module_name)
	: _level(level), _module_name(module_name), _stream(stream) {
#ifndef OTAWA_NOTRACE
		setStream(_stream);
#else
		setStream(elm::io::OutStream::null);
#endif
	}
	inline void checkLevel(int level) {
#ifndef OTAWA_NOTRACE
		if (level <= _level)
			setStream(_stream);
		else
			setStream(null_stream);
#endif
	}
};

inline Trace& operator<<(Trace& trace, const Level& level) {
	trace.checkLevel(level._level);
	return trace;
}

}

#endif // OTAWA_UTIL_TRACE_H
