/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::new_gliss::Process class interface
 */
#ifndef OTAWA_LOADER_NEW_GLISS_PROCESS_H
#define OTAWA_LOADER_NEW_GLISS_PROCESS_H

#include <otawa/prog/Process.h>

struct gel_line_map_t;
struct gel_file_info_t;

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
	virtual void get(Address at, signed char& val);
	virtual void get(Address at, unsigned char& val);
	virtual void get(Address at, signed short& val);
	virtual void get(Address at, unsigned short& val);
	virtual void get(Address at, signed long& val);
	virtual void get(Address at, unsigned long& val);
	virtual void get(Address at, signed long long& val);
	virtual void get(Address at, unsigned long long& val);
	virtual void get(Address at, Address& val);
	virtual void get(Address at, string& str);
	virtual void get(Address at, char *buf, int size);
	virtual Option<Pair<cstring, int> > getSourceLine(Address addr)
		throw (UnsupportedFeatureException);
	virtual void getAddresses(cstring file, int line,
		Vector<Pair<Address, Address> >& addresses)
		throw (UnsupportedFeatureException);
	
protected:
	friend class Segment;
	virtual Inst *decode(address_t addr) = 0;
	virtual void *gelFile(void) = 0;
	virtual void *memory(void) = 0;
	
public:
	void setup(void);
	Inst *_start; 
	hard::Platform *_platform;
	void *_state;
	void *_memory;
	int argc;
	char **argv, **envp;
	bool no_stack;
	bool init;
	struct gel_line_map_t *map;
	struct gel_file_info_t *file;
};

} } } // otawa::loader::new_gliss

#endif // OTAWA_LOADER_NEW_GLISS_PROCESS_H
