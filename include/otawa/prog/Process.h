/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/Process.h -- interface for Process class.
 */
#ifndef OTAWA_PROGRAM_PROCESS_H
#define OTAWA_PROGRAM_PROCESS_H

#include <elm/string.h>
#include <elm/Collection.h>
#include <elm/genstruct/Vector.h>
#include <otawa/instruction.h>

namespace otawa {

// Pre-definition
class File;
class Platform;
class Manager;
class CacheConfiguration;

// Process class
class Process: public ProgObject {
public:
	virtual ~Process(void) { };
	
	// Accessors
	virtual Platform *platform(void) = 0;
	virtual Manager *manager(void) = 0;
	virtual const CacheConfiguration& cache(void);
	virtual Inst *start(void) = 0;
	virtual Inst *findInstAt(address_t addr) = 0;
	virtual address_t findLabel(String& label);
	virtual Inst *findInstAt(String& label);

	// Constructors
	virtual elm::Collection<File *> *files(void) = 0;
	virtual File *createFile(void) = 0;
	virtual File *loadFile(elm::CString path) = 0;

};

} // otawa

#endif // OTAWA_PROG_PROCESS_H
