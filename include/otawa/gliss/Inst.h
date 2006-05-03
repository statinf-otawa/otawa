/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	Inst.h -- GLISS Inst class interface.
 */
#ifndef OTAWA_GLISS_INST_H
#define OTAWA_GLISS_INST_H

#include <otawa/instruction.h>
#define ISS_DISASM
#include <iss_include.h>

namespace otawa { namespace gliss {
	
// Class predefinition
class CodeSegment;
	
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
	const static unsigned long FLAG_Load = 0x10;
	const static unsigned long FLAG_Store = 0x20;
	const elm::genstruct::Table<hard::Register *> *reads;
	const elm::genstruct::Table<hard::Register *> *writes;
	
	void scan(void);
	virtual void scanCustom(instruction_t *inst) { };
	inline CodeSegment& segment(void) const { return seg; };
	void scanRegs(void);
public:
	inline Inst(CodeSegment& segment, address_t address)
		: seg(segment), addr(address), flags(0) { };
	virtual ~Inst(void);

	// Inst overload
	virtual address_t address(void);
	virtual size_t size(void);
	virtual void dump(io::Output& out);
	virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
};
	
} } // otawa::gliss

#endif // OTAWA_GLISS_INST_H
