/*
 *	$Id$
 *	Copyright (c) 2004-07, IRIT UPS.
 *
 *	GLISS CodeSegment class interface.
 */
#ifndef OTAWA_GLISS_CODESEGMENT_H
#define OTAWA_GLISS_CODESEGMENT_H

#include <otawa/program.h>

namespace otawa { namespace gliss {

using namespace elm::genstruct;

// Predefined classes
class File;
	
// CodeSegment class
class CodeSegment: public ::otawa::Segment {

public:
	CodeSegment(File& file, CString name, memory_t *memory, address_t address, size_t size);
	inline File& file(void) const { return _file; }

private:
	File& _file;
	void buildInsts(void);
	void buildLabs(void);
	memory_t *mem;
};

} } // otawa::gliss

#endif // OTAWA_GLISS_CODESEGMENT_H
