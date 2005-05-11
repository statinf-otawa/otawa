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
#include <otawa/instruction.h>

namespace otawa {

// Pre-definition
class File;
class Platform;
class Manager;

// Cache class
class Cache {
public:
	typedef enum policy_t {
		NONE = 0,
		LRU = 1,
		RANDOM = 2
	} policy_t;

	virtual int level(void) const = 0;
	virtual int size(void) const = 0;
	virtual int bits(void) const = 0;
	virtual int blockSize(void) const = 0;
	virtual int blockBits(void) const = 0;
	virtual int lineCount(void) const = 0;
	virtual int lineBits(void) const = 0;
};

// Process class
class Process: public ProgObject {
public:
	virtual ~Process(void) { };
	virtual elm::Collection<File *> *files(void) = 0;
	virtual File *createFile(void) = 0;
	virtual File *loadFile(elm::CString path) = 0;
	virtual Platform *platform(void) = 0;
	virtual Manager *manager(void) = 0;
	virtual Inst *start(void) = 0;
	virtual Inst *findInstAt(address_t addr) = 0;
	virtual address_t findLabel(String& label);
	virtual Inst *findInstAt(String& label);
};

} // otawa

#endif // OTAWA_PROG_PROCESS_H
