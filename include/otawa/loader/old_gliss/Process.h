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
	Process(
		Manager *manager,
		Loader *loader,
		hard::Platform *platform,
		const PropList& props = PropList::EMPTY);
	inline void *state(void) const { return _state; }

	// Process Overloads
	virtual hard::Platform *platform(void);
	virtual Inst *start(void);
	virtual File *loadFile(elm::CString path);
	virtual Loader *loader(void) const;
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
	virtual void *memory(void) = 0;

public:
	Inst *_start; 
	hard::Platform *_platform;
	void *_state;
	void *_memory;
	int argc;
	char **argv, **envp;
	Loader *_loader;
};

} } } // otawa::loader::old_gliss

#endif /* OTAWA_LOADER_OLD_GLISS_PROCESS_H */
