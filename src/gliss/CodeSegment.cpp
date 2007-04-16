/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	gliss/CodeSegment.cpp -- gliss::CodeSegment class implementation.
 */

#include <elm/debug.h>
#include <otawa/gliss.h>
#include <otawa/gliss/MemInst.h>

using namespace elm;

namespace otawa { namespace gliss {

/**
 * @class CodeSegment
 * A segment of code with the PPC Gliss loader.
 */

/**
 * Constructor.
 * @param file		Container file.
 * @param name	Name of the segment.
 * @param memory	Gliss memory.
 * @param address	Address of the segment in the simulator memory.
 * @param size			Size of the segment.
 */
CodeSegment::CodeSegment(
	File& file,
	CString name,
	memory_t *memory,
	address_t address,
	size_t size)
:
	Segment(name, address, size, Segment::EXECUTABLE),
	_file(file),
	mem(memory)
{
	buildInsts();
	buildLabs();
}


/**
 * Build the code and the instructions in the current segment.
 */
void CodeSegment::buildInsts(void) {
	code_t buffer[20];
	instruction_t *inst;
	
	// Build the instructions
	for(offset_t off = 0; off < size(); off += 4) {
		address_t addr = address() + off;
		
		// Get the instruction
		inst = 0;
		iss_fetch((::address_t)addr, buffer);
		inst = iss_decode(_file.state(), (::address_t)addr, buffer, 0);
		assert(inst);
	
		// Look for its kind
		Inst *result;
		if(inst->ident == ID_Instrunknown)
			result = new Inst(*this, addr);
		else {
			assert(iss_table[inst->ident].category <= 26);
			switch(iss_table[inst->ident].category) {
			case 5: 	// STORE
			case 6:		// LOAD
			case 22:	// FPLOAD
			case 23:	// FPSTORE
				result = new MemInst(*this, addr);
				break;
			case 8:		// BRANCH
			case 10:	// SYSTEM
			case 11:	// TRAP
				if(inst->ident == ID_BL_ && inst->instrinput[0].val.Int24 == 1)
					result = new Inst(*this, addr);
				else
					result = new ControlInst(*this, addr);
				break;
			
			default:
				result = new Inst(*this, addr);
				break;
			}
		}
				
		// Cleanup
		insert(result);
		iss_free(inst);
	}
}


/**
 * Build the labels.
 * 
 */
void CodeSegment::buildLabs(void) {

	// Add symbols
	address_t lbound = address(), ubound = lbound + size();
	for(File::SymIter sym(&_file); sym; sym++) {
		//cerr << "Symbol " << sym->name() << io::endl;
		address_t addr = sym->address();
		if(addr >= lbound && addr < ubound) {
			Inst *inst = (Inst *)findInstAt(addr);
			if(inst) {
				//cerr << "==> " << sym->address() << " = " << inst->address() << io::endl; 
				//Identifier *id;
				Symbol::ID(inst) = sym;
				switch(sym->kind()) {
				case Symbol::FUNCTION:
					FUNCTION_LABEL(inst).add(sym->name());
				case Symbol::LABEL:
					LABEL(inst).add(sym->name());
					break;
				}
			}
			else
				cerr << "WARNING: no matching instruction for label "
					 << sym->name() << io::endl;
		} 
	}
}

} } // otawa::gliss
