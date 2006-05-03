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


/**
 */
Inst::~Inst(void) {
	if(reads && reads != &no_regs)
		delete (elm::genstruct::AllocatedTable<hard::Register *> *)reads;
	if(writes && writes != &no_regs)
		delete (elm::genstruct::AllocatedTable<hard::Register *> *)writes;
}


// Overloaded
address_t Inst::address(void) {
	return addr;
}


// Overloaded
size_t Inst::size(void) {
	return 4;
}


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


// Overload
const elm::genstruct::Table<hard::Register *>& Inst::readRegs(void) {
	if(!reads)
		scanRegs();
	return *reads;
}


// Overload
const elm::genstruct::Table<hard::Register *>& Inst::writtenRegs(void) {
	if(!writes)
		scanRegs();
	return *writes;
}


/**
 * Scan the register used by the statement.
 */
void Inst::scanRegs(void) {

	// Decode instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)addr, buffer);
	inst = iss_decode((::address_t)(unsigned long)addr, buffer);
	
	// Get read registers
	int cnt = 0;
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(inst->instrinput[i].type == GPR_T
		|| inst->instrinput[i].type == FPR_T)
			cnt++;
	elm::genstruct::AllocatedTable<hard::Register *> *tab =
		new elm::genstruct::AllocatedTable<hard::Register *>(cnt);
	cnt = 0;
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(inst->instrinput[i].type == GPR_T)
			tab->set(cnt++, Platform::GPR_bank[inst->instrinput[i].val.Uint5]);
		else if(inst->instrinput[i].type == FPR_T)
			tab->set(cnt++, Platform::FPR_bank[inst->instrinput[i].val.Uint5]);
	reads = tab;

	// Get write registers
	cnt = 0;
	for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(inst->instroutput[i].type == GPR_T
		|| inst->instroutput[i].type == FPR_T)
			cnt++;
	tab = new elm::genstruct::AllocatedTable<hard::Register *>(cnt);
	cnt = 0;
	for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(inst->instroutput[i].type == GPR_T)
			tab->set(cnt++, Platform::GPR_bank[inst->instroutput[i].val.Uint5]);
		else if(inst->instroutput[i].type == FPR_T)
			tab->set(cnt++, Platform::FPR_bank[inst->instroutput[i].val.Uint5]);
	writes = tab;
	
	// Free instruction
	iss_free(inst);
}

} } // otawa::gliss
