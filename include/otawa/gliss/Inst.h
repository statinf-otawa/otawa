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
	
} } // otawa::gliss

#endif // OTAWA_GLISS_INST_H
