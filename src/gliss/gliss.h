/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss.h -- GLISS classes interface.
 */
#ifndef OTAWA_GLISS_GLISS_H
#define OTAWA_GLISS_GLISS_H

#include <elm/io.h>
#include <elm/genstruct/HashTable.h>
#include <elm/datastruct/Vector.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include <otawa/instruction.h>
#define ISS_DISASM
#include <iss_include.h>
#include <elfread.h>

namespace otawa { namespace gliss {

// Classes
class Inst;
class ControlInst;
class Platform;
class CodeSegment;
class DataSegment;
class File;

// Inst class
class Inst: public otawa::Inst {
	CodeSegment& seg;
	address_t addr;
protected:
	unsigned long flags;
	const static unsigned long FLAG_Built = 0x01;
	const static unsigned long FLAG_Cond = 0x02;
	const static unsigned long FLAG_Call = 0x04;
	const static unsigned long FLAG_Return = 0x08;
	void scan(void);
	virtual void scanCustom(instruction_t *inst) { };
	inline CodeSegment& segment(void) const { return seg; };
public:
	inline Inst(CodeSegment& segment, address_t address)
		: seg(segment), addr(address), flags(0) { }; 

	// Inst overload
	virtual address_t address(void);
	virtual size_t size(void);
	virtual void dump(io::Output& out);
	virtual Collection<Operand *> *getOps(void);
	virtual Collection<Operand *> *getReadOps(void);
	virtual Collection<Operand *> *getWrittenOps(void);
};


// ControlInst class
class ControlInst: public Inst {
	Inst *_target;
	virtual void scanCustom(instruction_t *inst);
public:
	inline ControlInst(CodeSegment& segment, address_t address)
		: Inst(segment, address) { }; 

	// Inst overload	
	virtual bool isControl(void);
	virtual bool isBranch(void);
	virtual bool isCall(void);
	virtual bool isReturn(void);
	virtual void dump(io::Output& out);

	// ControlInst overload
	virtual bool isConditional(void);
	virtual otawa::Inst *target(void);
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
		inhstruct::DLList insts;
		memory_t *mem;
		address_t addr;
		size_t _size;

		Code(memory_t *memory, address_t address, size_t size);
		~Code(void);
		CString name(void);
		address_t address(void);
		size_t size(void);
		virtual Inst *first(void) const;
		virtual Inst *last(void) const;
	};

	// attributes
	File& file;
	String _name;
	Code code;
	bool built;
	datastruct::Vector<ProgItem *> _items;
	void build(void);

public:
	CodeSegment(File& _file, CString name, memory_t *memory, address_t address, size_t size);
	otawa::Inst *findByAddress(address_t addr);

	// Segment overload
	virtual CString name(void);
	virtual int flags(void);
	virtual address_t address(void);
	virtual size_t size(void);
	virtual datastruct::Collection<ProgItem *>& items(void);
};


// DataSegment class
class DataSegment: public ::otawa::Segment {
};


// File class
class File: public ::otawa::File {
	friend class CodeSegment;
	String path;
	elm::datastruct::Vector<Segment *> segs;
	state_t *state;
	genstruct::HashTable<String, address_t> labels;
public:
	File(String _path, int argc, char **argv, char **envp);
	~File(void);
	inline bool isOK(void) const { return !segs.isEmpty(); };
	otawa::Inst *findByAddress(address_t addr);

	// ::otawa::File overload
	virtual CString name(void);
	virtual const elm::datastruct::Collection<Segment *>& segments(void) const;
	virtual address_t findLabel(const String& label);
};


// Process class
class Process: public ::otawa::Process {
	elm::datastruct::Vector<otawa::File *> _files;
	Manager *man;
	int argc;
	char **argv, **envp;
	address_t start_addr;
public:
	Process(Manager *_man, PropList& props);
	virtual ~Process(void);
	void clear(void);

	// elm::Process overload
	virtual const elm::datastruct::Collection<otawa::File *> *files(void) const;
	virtual ::otawa::File *createFile(void);
	virtual ::otawa::File *loadFile(CString path);
	virtual ::otawa::Platform *platform(void);
	virtual ::otawa::Manager *manager(void);
	virtual otawa::Inst *start(void);
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
