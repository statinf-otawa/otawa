/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss.h -- GLISS classes interface.
 */
#ifndef OTAWA_GLISS_GLISS_H
#define OTAWA_GLISS_GLISS_H

#include <elm/io.h>
#include <elm/datastruct/HashTable.h>
#include <elm/datastruct/Vector.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include <otawa/instruction.h>
#include <elfread.h>
#include <otawa/gliss/Inst.h>
#include <otawa/gliss/ControlInst.h>
#include <otawa/gliss/CodeSegment.h>
#include <otawa/gliss/File.h>
#include <otawa/gliss/Process.h>

namespace otawa { namespace gliss {

// Classes
class Inst;
class ControlInst;
class Platform;
class CodeSegment;
class DataSegment;
class File;

// Platform class
class Platform: public ::otawa::Platform {
public:
	static Platform platform;
};


// DataSegment class
class DataSegment: public ::otawa::Segment {
};


// otawa::gliss::Loader class
class Loader: public otawa::Loader {
public:

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *load(Manager *_man, CString path, PropList& props);
	virtual otawa::Process *create(Manager *_man, PropList& props);
};


// Loader entry point
extern otawa::Loader& loader;


} }	// otawa::gliss

#endif	// OTAWA_GLISS_GLISS_H
