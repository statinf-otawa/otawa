/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/Process.h -- interface for heptane::Process class.
 */
#ifndef OTAWA_HEPTANE_PROCESS_H
#define OTAWA_HEPTANE_PROCESS_H

#include <gliss.h>

namespace otawa { namespace heptane {

// heptane::Process class
class Process: public gliss::Process {
public:
	Process(Manager *_man, PropList& props);
	
	// Process overload
	virtual ::otawa::File *loadFile(CString path);
};
	
} }	// otawa::heptane

#endif	// OTAWA_HEPTANE_PROCESS_H
