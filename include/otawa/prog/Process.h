/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	otawa/prog/Process.h -- interface for Process class.
 */
#ifndef OTAWA_PROGRAM_PROCESS_H
#define OTAWA_PROGRAM_PROCESS_H

#include <elm/string.h>
#include <elm/system/Path.h>
#include <elm/genstruct/Vector.h>
#include <otawa/instruction.h>
#include <otawa/program.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa {

using namespace elm::genstruct;

// Pre-definition
class File;
class Manager;
class Processor;
class TextDecoder;

namespace hard {
	class Platform;
	class CacheConfiguration;
}

namespace sim {
	class Simulator;
}


// Process class
class Process: public PropList {
	Vector<File *> files;
	File *prog;
	Manager *man;

protected:
	void addFile(File *file);

public:
	Process(Manager *manager, const PropList& props = EMPTY, File *program = 0);
	virtual ~Process(void);
	
	// Accessors
	virtual hard::Platform *platform(void) = 0;
	inline Manager *manager(void) { return man; }
	virtual const hard::CacheConfiguration& cache(void);
	virtual Inst *start(void) = 0;
	virtual Inst *findInstAt(address_t addr);
	virtual address_t findLabel(String& label);
	virtual Inst *findInstAt(String& label);
	virtual sim::Simulator *simulator(void);
	inline File *program(void) const;
	virtual int instSize(void) const = 0;
	virtual Processor *decoder(void);

	// Constructors
	File *loadProgram(elm::CString path);
	virtual File *loadFile(elm::CString path) = 0;
	
	// FileIterator 
	class FileIter: public Vector<File *>::Iterator {
	public:
		inline FileIter(const Process *process)
			: Vector<File *>::Iterator(process->files) { }
		inline FileIter(const FileIter& iter)  
			: Vector<File *>::Iterator(iter) { }
	};

};

// Inlines
inline File *Process::program(void) const {
	return prog;
}

// Features
//extern Feature<NoProcessor> DECODED_TEXT;

} // otawa

#endif // OTAWA_PROG_PROCESS_H
