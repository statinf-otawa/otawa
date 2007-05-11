/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::Inst class implementation
 */

#include <elm/assert.h>
#include <otawa/loader/old_gliss/Inst.h>
#include "old_gliss.h"

namespace otawa { namespace loader { namespace old_gliss {


/**
 * @class Inst
 * Generic instruction used in old_gliss kit.
 */


/**
 * Build an instruction.
 * @param kind		Kind of the instruction.
 * @param process	Owner process.
 * @param address	Instruction address.
 */
Inst::Inst(Process& process, kind_t kind, address_t address)
: proc(process), _kind(kind), addr(address) {
}


/**
 * @fn Process& Inst::process(void) const;
 * Get the owner process.
 * @return	Owner process.
 */


/**
 */
void Inst::dump(io::Output& out) {
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode(process().state(), (::address_t)addr, buffer);
	iss_disasm(out_buffer, inst);
	out << out_buffer;
	iss_free(inst);
}


/**
 */
Inst::kind_t Inst::kind(void) {
	return _kind & ~MASK;
}


/**
 */
const elm::genstruct::Table<hard::Register *>& Inst::readRegs(void) {
	if(!(_kind & REGS_DONE)) {
		_kind |= REGS_DONE;
		decodeRegs();
	}
	return in_regs;
}


/**
 */
const elm::genstruct::Table<hard::Register *>& Inst::writtenRegs(void) {
	if(!(_kind & REGS_DONE)) {
		_kind |= REGS_DONE;
		decodeRegs();
	}
	return out_regs;
}


/**
 */
address_t Inst::address(void) const {
	return addr;
}


/**
 * This method is called when the registers need to be decoded. It must
 * overriden by specialization of this class that must initialize the fields
 * in_regs and out_regs.
 */
void Inst::decodeRegs(void)  {
}

} } } // otawa::loader
