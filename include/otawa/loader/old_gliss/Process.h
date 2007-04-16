/*
 *	$Id$
 *	Copyright (c) 2004-07, IRIT UPS <casse@irit.fr>
 *
 *	loader::old_gliss::Process class interface
 */
#ifndef OTAWA_LOADER_OLD_PROCESS_H
#define OTAWA_LOADER_OLD_PROCESS_H

#include <otawa/prog/Process.h>

namespace otawa {
	
namespace loader { namespace old_gliss {

class Segment;

// Process class
class Process: public otawa::Process {
public:
	Process(Manager *manager, hard::Platform *platform,
		const PropList& props = PropList::EMPTY);
	inline void *state(void) const { return _state; }

	// Process Overloads
	virtual hard::Platform *platform(void);
	virtual Inst *start(void);
	virtual File *loadFile(elm::CString path);

protected:
	friend class Segment;
	virtual Inst *decode(address_t addr) = 0;
	
public:
	Inst *_start; 
	hard::Platform *_platform;
	void *_state;
	int argc;
	char **argv, **envp;
};

} } } // otawa::loader::old_gliss

#endif /* OTAWA_LOADER_OLD_GLISS_PROCESS_H */
