/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::new_gliss::Process class interface
 */
#ifndef OTAWA_LOADER_NEW_GLISS_PROCESS_H
#define OTAWA_LOADER_NEW_GLISS_PROCESS_H

#include <otawa/prog/Process.h>

namespace otawa {
	
namespace loader { namespace new_gliss {

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
	virtual void *gelFile(void) = 0;
	
public:
	Inst *_start; 
	hard::Platform *_platform;
	void *_state;
	int argc;
	char **argv, **envp;
	bool no_stack;
};

} } } // otawa::loader::new_gliss

#endif // OTAWA_LOADER_NEW_GLISS_PROCESS_H
