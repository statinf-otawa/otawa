/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss.h -- GLISS classes interface.
 */
#ifndef OTAWA_GLISS_GLISS_H
#define OTAWA_GLISS_GLISS_H

#include <elm/datastruct/Vector.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include <otawa/instruction.h>
#include <emul.h>
#include <elfread.h>

// Power instruction kinds
#define OTAWA_COMP 0
#define OTAWA_LOAD 64
#define OTAWA_STORE 65
#define OTAWA_BRANCH_REL 128
#define OTAWA_BRANCH_ABS 129
#define OTAWA_BRANCH_LINK 130
#define OTAWA_BRANCH_LINK_ABS 131
#define OTAWA_BRANCH_COND_MEM 132
#define OTAWA_BRANCH_COND_REL 133
#define OTAWA_BRANCH_COND_ABS 134
#define OTAWA_BRANCH_COND_LINK 135
#define OTAWA_BRANCH_COND_LINK_ABS 136
#define OTAWA_BRANCH_COND_CTR 137
#define OTAWA_BRANCH_COND_CTR_LINK 138
#define OTAWA_BRANCH_COND_LR 139
#define OTAWA_BRANCH_COND_LR_LINK 140
#define OTAWA_SYSCALL 196


namespace otawa { namespace gliss {

// Classes
class Inst;
class ControlInst;
class Platform;
class CodeSegment;
class DataSegment;

// Inst class
class Inst: public otawa::Inst {
	CodeSegment& seg;
	address_t addr;
protected:
	unsigned long flags;
	const static unsigned long FLAG_Built = 0x01;
	const static unsigned long FLAG_Cond = 0x02;
	const static unsigned long FLAG_Call = 0x04;
	const static unsigned long FLAG_Return = 0x8;
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
	datastruct::Vector<ProgItem *> _items;
	void build(void);

public:
	CodeSegment(CString name, memory_t *memory, address_t address, size_t size);
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
	String path;
	elm::datastruct::Vector<Segment *> segs;
	state_t *state;
public:
	File(String _path);
	~File(void);
	inline bool isOK(void) const { return !segs.isEmpty(); };

	// ::otawa::File overload
	CString name(void);
	const elm::datastruct::Collection<Segment *>& segments(void) const;
};


// Process class
class Process: public ::otawa::Process {
	elm::datastruct::Vector<otawa::File *> _files;
	Manager *man;
public:
	Process(Manager *_man);
	void clear(void);

	// elm::Process overload
	virtual const elm::datastruct::Collection<otawa::File *> *files(void) const;
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
