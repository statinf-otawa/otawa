/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	gliss/Process.h -- gliss::Process class interface.
 */
#ifndef OTAWA_GLISS_PROCESS_H
#define OTAWA_GLISS_PROCESS_H

#include <otawa/manager.h>
#include <otawa/program.h>

namespace otawa { namespace gliss {

// External classes
class Platform;

// Process class
class Process: public otawa::Process {
	elm::datastruct::Vector<otawa::File *> _files;
protected:
	Manager *man;
	int argc;
	char *default_argv[2];
	char *default_envp[1];
	char **argv, **envp;
	address_t start_addr;
	Platform *_platform;
public:
	Process(Manager *_man, PropList& props);
	virtual ~Process(void);
	void clear(void);

	// elm::Process overload
	virtual elm::Collection<otawa::File *> *files(void);
	virtual ::otawa::File *createFile(void);
	virtual ::otawa::File *loadFile(CString path);
	virtual hard::Platform *platform(void);
	virtual ::otawa::Manager *manager(void);
	virtual otawa::Inst *start(void);
	virtual otawa::Inst *findInstAt(address_t addr);
};

} } // otawa::gliss

#endif // OTAWA_GLISS_PROCESS_H
