/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss.h -- GLISS classes interface.
 */
#ifndef OTAWA_GLISS_GLISS_H
#define OTAWA_GLISS_GLISS_H

#include <elm/obj/vector.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include <otawa/instruction.h>
#include <emul.h>
#include <elfread.h>

namespace otawa { namespace gliss {

// Classes
class Inst;
class Platform;
class CodeSegment;
class DataSegment;

// Inst class
class Inst: public otawa::Inst {
	CodeSegment& seg;
	address_t addr;
	bool built;
public:
	inline Inst(CodeSegment& segment, address_t address): seg(segment), addr(address), built(false) { }; 

	// Inst overload
	virtual address_t address(void);
	virtual size_t size(void);
	virtual void dump(io::Output& out);
	virtual Collection<Operand *> *getOps(void);
	virtual Collection<Operand *> *getReadOps(void);
	virtual Collection<Operand *> *getWrittenOps(void);
};


// Platform class
class Platform: public ::otawa::Platform {
public:
	static Platform platform;
};


// CodeSegment class
class CodeSegment: public ::otawa::Segment {

	// Code representation
	class Code: public otawa::Code {
	public:
		memory_t *mem;
		address_t addr;
		size_t _size;

		inline Code(memory_t *memory, address_t address, size_t size): mem(memory), addr(address), _size(size) { };
		CString name(void);
		address_t address(void);
		size_t size(void);
	};

	// attributes
	String _name;
	Code code;
	bool built;
	obj::Vector<ProgItem *> _items;
	void build(void);

public:
	CodeSegment(CString name, memory_t *memory, address_t address, size_t size);

	// Segment overload
	virtual CString name(void);
	virtual int flags(void);
	virtual address_t address(void);
	virtual size_t size(void);
	virtual Sequence<ProgItem *>& items(void);
};


// DataSegment class
class DataSegment: public ::otawa::Segment {
};


// File class
class File: public ::otawa::File {
	String path;
	elm::obj::Vector<Segment *> segs;
	state_t *state;
public:
	File(String _path);
	~File(void);
	inline bool isOK(void) const { return !segs.isEmpty(); };

	// ::otawa::File overload
	CString name(void);
	const elm::Collection<Segment *>& segments(void) const;
};


// Process class
class Process: public ::otawa::Process {
	elm::obj::Vector<otawa::File *> _files;
	Manager *man;
public:
	Process(Manager *_man);
	void clear(void);

	// elm::Process overload
	virtual const elm::Collection<otawa::File *> *files(void) const;
	virtual ::otawa::File *createFile(void);
	virtual ::otawa::File *loadFile(CString path);
	virtual ::otawa::Platform *platform(void) const;
	virtual ::otawa::Manager *manager(void) const;
};


// otawa::gliss::Loader class
class Loader: public otawa::Loader {
public:

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual Process *load(Manager *_man, CString path, PropList& props);
	virtual Process *create(Manager *_man, PropList& props);
};


// Loader entry point
extern otawa::Loader& loader;


} }	// otawa::gliss

#endif	// OTAWA_GLISS_GLISS_H

