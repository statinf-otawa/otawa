/*
 *	$Id$
 *	Process class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-7, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_PROGRAM_PROCESS_H
#define OTAWA_PROGRAM_PROCESS_H

#include <elm/string.h>
#include <elm/system/Path.h>
#include <elm/genstruct/Vector.h>
#include <otawa/instruction.h>
#include <otawa/program.h>
#include <otawa/proc/Feature.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa {

using namespace elm::genstruct;

// Pre-definition
class File;
namespace hard {
	class Platform;
	class CacheConfiguration;
}
class Loader;
class Manager;
class Processor;
class Process;
namespace sim {
	class Simulator;
}
class Symbol;
class TextDecoder;


// SimState class
class SimState {
public:
	SimState(Process *process);
	virtual ~SimState(void);
	inline Process *process(void) const { return proc; }
	virtual Inst *execute(Inst *inst) = 0;

private:
	Process *proc;
};

// Process class
class Process: public PropList {
	Vector<File *> files;
	Vector<AbstractFeature *> provided;
	File *prog;
	Manager *man;
	void link(WorkSpace *ws);
	void unlink(WorkSpace *ws);

protected:
	friend class WorkSpace;
	void addFile(File *file);
	void provide(AbstractFeature& feature);

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
	inline File *program(void) const { return prog; }
	virtual int instSize(void) const = 0;
	virtual Processor *decoder(void);
	virtual Loader *loader(void) const;
	Symbol *findSymbol(const string& name); 	

	// Memory access
	virtual void get(Address at, signed char& val);
	virtual void get(Address at, unsigned char& val);
	virtual void get(Address at, signed short& val);
	virtual void get(Address at, unsigned short& val);
	virtual void get(Address at, signed long& val);
	virtual void get(Address at, unsigned long& val);
	virtual void get(Address at, signed long long& val);
	virtual void get(Address at, unsigned long long& val);
	virtual void get(Address at, Address& val);
	virtual void get(Address at, float& val);
	virtual void get(Address at, double& val);
	virtual void get(Address at, long double& val);
	virtual void get(Address at, string& str);
	virtual void get(Address at, char *buf, int size);
	
	// Simulation management
	virtual SimState *newState(void);
	virtual sim::Simulator *simulator(void);

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


// Process display
elm::io::Output& operator<<(elm::io::Output& out, Process *proc); 


// ProcessException class
class ProcessException: public Exception {
public:
	inline ProcessException(Process *process): proc(process)
		{ ASSERTP(process, "null process passed"); }
	inline Process *process(void) const { return proc; }
private:
	Process *proc;
};


// UnsupportedFeatureException class
class UnsupportedFeatureException: public ProcessException {
public:
	
 	inline UnsupportedFeatureException(
 		Process *proc,
 		const AbstractFeature& feature
 	): ProcessException(proc), f(feature) { }
 		
 	 inline UnsupportedFeatureException(const AbstractFeature& feature)
 	 : ProcessException(0), f(feature) { }
 	 		
 	 inline const AbstractFeature& feature(void) const { return f; }
 	virtual String 	message(void); 
private:
	const AbstractFeature& f;
};


// OutOfSegmentException class
class OutOfSegmentException: public ProcessException { 
public:
	OutOfSegmentException(Process *proc, Address address)
		: ProcessException(proc), addr(address) { }
	inline Address address(void) const { return addr; }
 	virtual String 	message(void); 	
private:
	Address addr;
};

// Features
extern Feature<NoProcessor> MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> FLOAT_MEMORY_ACCESS_FEATURE;
extern Feature<NoProcessor> STACK_USAGE_FEATURE;
extern Feature<NoProcessor> REGISTER_USAGE_FEATURE;

} // otawa

#endif // OTAWA_PROG_PROCESS_H
