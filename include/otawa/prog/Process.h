/*
 *	$Id$
 *	Process class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-8, IRIT UPS.
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

#include <elm/data/List.h>
#include <elm/data/Vector.h>
#include <elm/stree/Tree.h>
#include <elm/string.h>
#include <elm/sys/Path.h>
#include <elm/util/LockPtr.h>

#include <otawa/instruction.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/features.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa {

// Pre-definition
class File;
namespace hard {
	class Platform;
	class CacheConfiguration;
	class Register;
}
class Loader;
class Manager;
class Processor;
class Process;
namespace sem { class Block; }
namespace sim { class Simulator; }
class Symbol;
class TextDecoder;


// ProcessException class
class ProcessException: public Exception {
public:
	inline ProcessException(void): proc(nullptr) { }
	inline ProcessException(Process *process): proc(process)
		{ ASSERTP(process, "null process passed"); }
	inline ProcessException(Process *process, const string& message): Exception(message), proc(process)
		{ ASSERTP(process, "null process passed"); }
	inline Process *process(void) const { return proc; }
 	virtual String message(void);

private:
	Process *proc;
};


// UnsupportedFeatureException class
class UnsupportedFeatureException: public ProcessException {
public:
	inline UnsupportedFeatureException(const AbstractFeature& feature): f(feature) { }
	inline UnsupportedFeatureException(Process *p, const AbstractFeature& feature): ProcessException(p), f(feature) { }
	inline const AbstractFeature& feature(void) const { return f; }
 	virtual String message(void);
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


// DecodingException class
class DecodingException: public elm::MessageException {
public:
	DecodingException(const string& message);
};


// SimState class
class SimState {
public:
	SimState(Process *process);
	virtual ~SimState(void);
	inline Process *process(void) const { return proc; }
	virtual Inst *execute(Inst *inst) = 0;

	// register access
	virtual void setSP(const Address& addr);
	virtual t::uint32 getReg(hard::Register *r);
	virtual void setReg(hard::Register *r, t::uint32 v);

	// memory accesses
	virtual Address lowerRead(void);
	virtual Address upperRead(void);
	virtual Address lowerWrite(void);
	virtual Address upperWrite(void);

private:
	Process *proc;
};



// Process class
class Process: public PropList, public Lock {
public:
	static rtti::Type& __type;

	Process(Manager *manager, const PropList& props = EMPTY, File *program = 0);
	virtual ~Process(void);
	inline const List<AbstractFeature *>& features(void) const { return provided; }
	inline const Vector<File *>& files(void) const { return _files; }

	// Accessors
	virtual hard::Platform *platform(void) = 0;
	inline Manager *manager(void) { return man; }
	virtual Inst *start(void) = 0;
	virtual Inst *findInstAt(address_t addr);
	virtual address_t findLabel(const string& label);
	virtual Inst *findInstAt(const string& label);
	inline File *program(void) const { return prog; }
	virtual int instSize(void) const = 0;
	virtual Processor *decoder(void);
	virtual Loader *loader(void) const;
	Symbol *findSymbol(const string& name);
	Symbol *findSymbolAt(const Address& address);
	virtual Address initialSP(void) const;
	virtual Inst *newNOp(Address addr = Address::null);
	virtual void deleteNop(Inst *inst);
	virtual int maxTemp(void) const;
	Segment *findSegmentAt(Address addr) const;

	// Memory access
	virtual void get(Address at, t::int8& val);
	virtual void get(Address at, t::uint8& val);
	virtual void get(Address at, t::int16& val);
	virtual void get(Address at, t::uint16& val);
	virtual void get(Address at, t::int32& val);
	virtual void get(Address at, t::uint32& val);
	virtual void get(Address at, t::int64& val);
	virtual void get(Address at, t::uint64& val);
	virtual void get(Address at, Address& val);
	virtual void get(Address at, float& val);
	virtual void get(Address at, double& val);
	virtual void get(Address at, long double& val);
	virtual void get(Address at, string& str);
	virtual void get(Address at, char *buf, int size);

	// LineNumber feature
	virtual Option<Pair<cstring, int> > getSourceLine(Address addr);
	virtual void getAddresses(cstring file, int line, Vector<Pair<Address, Address> >& addresses);

	// Simulation management
	virtual SimState *newState(void);
	virtual sim::Simulator *simulator(void);

	// Constructors
	File *loadProgram(elm::CString path);
	virtual File *loadFile(elm::CString path) = 0;

	// loader 1.2.0
	virtual Address defaultStack(void);
	virtual void semInit(sem::Block& block);

	// FileIterator
	class FileIter: public Vector<File *>::Iter {
	public:
		inline FileIter(const Process *process): Vector<File *>::Iter(process->_files) { }
	};

protected:
	friend class WorkSpace;
	void addFile(File *file);
	void provide(AbstractFeature& feature);

private:
	Vector<File *> _files;
	List<AbstractFeature *> provided;
	File *prog;
	Manager *man;
	stree::Tree<Address::offset_t, Symbol *> *smap;
};


// Process display
elm::io::Output& operator<<(elm::io::Output& out, Process *proc);

} // otawa

#endif // OTAWA_PROG_PROCESS_H
