/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/Process.h -- interface for Process class.
 */
#ifndef OTAWA_PROGRAM_PROCESS_H
#define OTAWA_PROGRAM_PROCESS_H

#include <elm/string.h>
#include <elm/system/Path.h>
#include <elm/Collection.h>
#include <elm/genstruct/Vector.h>
#include <otawa/instruction.h>
#include <otawa/program.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa {

// Pre-definition
class File;
class Manager;

namespace hard {
	class Platform;
	class CacheConfiguration;
}

namespace sim {
	class Simulator;
}


// Process class
class Process: public ProgObject {
	File *prog;
public:
	Process(const PropList& props = EMPTY, File *program = 0);
	virtual ~Process(void) { };
	
	// Accessors
	virtual hard::Platform *platform(void) = 0;
	virtual Manager *manager(void) = 0;
	virtual const hard::CacheConfiguration& cache(void);
	virtual Inst *start(void) = 0;
	virtual Inst *findInstAt(address_t addr) = 0;
	virtual address_t findLabel(String& label);
	virtual Inst *findInstAt(String& label);
	virtual elm::Collection<File *> *files(void) = 0;
	virtual sim::Simulator *simulator(void);
	inline File *program(void) const;
	elm::xom::Element *config(void);
	void loadConfig(const elm::system::Path& path);

	// Constructors
	virtual File *createFile(void) = 0;
	File *loadProgram(elm::CString path);
	virtual File *loadFile(elm::CString path) = 0;

};

// Inlines
inline File *Process::program(void) const {
	return prog;
}

} // otawa

#endif // OTAWA_PROG_PROCESS_H
