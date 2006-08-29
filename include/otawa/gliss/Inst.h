/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
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
	const static unsigned long BUILT = 0x10000000;
	const elm::genstruct::Table<hard::Register *> *reads;
	const elm::genstruct::Table<hard::Register *> *writes;
	
	void scan(void);
	virtual void scanCustom(instruction_t *inst) { };
	inline CodeSegment& segment(void) const { return seg; };
	void scanRegs(void);
public:
	Inst(CodeSegment& segment, address_t address);
	virtual ~Inst(void);

	// Inst overload
	virtual address_t address(void);
	virtual size_t size(void);
	virtual void dump(io::Output& out);
	virtual kind_t kind(void);
	virtual const elm::genstruct::Table<hard::Register *>& readRegs(void);
	virtual const elm::genstruct::Table<hard::Register *>& writtenRegs(void);
};
	
} } // otawa::gliss

#endif // OTAWA_GLISS_INST_H
