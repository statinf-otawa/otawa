/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/Instruction.cpp -- gliss::Instruction class implementation.
 */

#include <elm/io.h>
#include <otawa/gliss.h>

using namespace elm;

namespace otawa { namespace gliss {

	
/**
 * @class Inst
 * Representation of instructions in Gliss.
 */

	
// Overloaded
address_t Inst::address(void) {
	return addr;
}


// Overloaded
size_t Inst::size(void) {
	return 4;
}


// Overloaded
/*Collection<Operand *> *Inst::getOps(void) {
	return 0;	// !!TODO!!
}*/


// Overloaded
/*Collection<Operand *> *Inst::getReadOps(void) {
	return 0;	// !!TODO!!
}*/


// Overloaded
/*Collection<Operand *> *Inst::getWrittenOps(void) {
	return 0;	// !!TODO!!
}*/


// Overloaded
void Inst::dump(io::Output& out) {
	
	// Display the hex value
	
	
	// Disassemble the statement
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)addr, buffer);
	inst = iss_decode((::address_t)(unsigned long)addr, buffer);
	iss_disasm(out_buffer, inst);
	out << out_buffer;
	iss_free(inst);
}


/**
 * Scan the instruction for filling the object.
 */
void Inst::scan(void) {

	// Already computed?
	if(flags & FLAG_Built)
		return;
	
	// Get the instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)address(), buffer);
	inst = iss_decode((::address_t)(unsigned long)address(), buffer);
	assert(inst);
	
	// Call customization
	scanCustom(inst);
	
	// Cleanup
	iss_free(inst);
	flags |= FLAG_Built;
}


} } // otawa::gliss
