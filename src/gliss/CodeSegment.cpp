/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/CodeSegment.cpp -- gliss::CodeSegment class implementation.
 */

#include <otawa/gliss.h>
#include <otawa/gliss/Symbol.h>

namespace otawa { namespace gliss {


/**
 * @class CodeSegment
 * A segment of code with the PPC Gliss loader.
 */

/**
 * Constructor.
 * @param _file		Container file.
 * @param name	Name of the segment.
 * @param memory	Gliss memory.
 * @param address	Address of the segment in the simulator memory.
 * @param size			Size of the segment.
 */
CodeSegment::CodeSegment(File& _file, CString name, memory_t *memory, address_t address, size_t size)
: file(_file), _name(name), code(memory, address, size), built(false) {
}

// Overloaded
CString CodeSegment::name(void) {
	return _name.toCString();
}

// Overloaded
address_t CodeSegment::address(void) {
	return code.addr;
}

// Overloaded
size_t CodeSegment::size(void) {
	return code._size;
}

// Overloaded
Collection<ProgItem *>& CodeSegment::items(void) {
	if(!built)
		build();
	return _items;
}

// Overloaded
int CodeSegment::flags(void) {
	return EXECUTABLE;
}

/**
 * Build the code and the instructions in the current segment.
 */
void CodeSegment::build(void) {
	code_t buffer[20];
	instruction_t *inst;
	
	// Link the code
	built = true;
	_items.add(&code);
	
	// Build the instructions
	for(offset_t off = 0; off < code._size; off += 4) {
		address_t addr = code.addr + off;
		
		// Get the instruction
		inst = 0;
		iss_fetch((::address_t)(unsigned long)addr, buffer);
		inst = iss_decode((::address_t)(unsigned long)addr, buffer);
		assert(inst);
	
		// Look for its kind
		switch(inst->ident) {
		case ID_BL_:
			if(inst->instrinput[0].val.Int24 == 1) {
				code.insts.addLast(new Inst(*this, addr));
				break;
			}
		case ID_B_:
		case ID_BA_:
		case ID_BLA_:
		case ID_BC_:
		case ID_BCA_:
		case ID_BCL_:
		case ID_BCLA_:
		case ID_BCCTR_:
		case ID_BCCTRL_:
		case ID_BCLR_:
		case ID_BCLRL_:
		case ID_SC:
			code.insts.addLast(new ControlInst(*this, addr));
			break;
		default:
			code.insts.addLast(new Inst(*this, addr));
			break;
		}
		
		// Cleanup
		iss_free(inst);
	}
	
	// Read the symbols
	Elf32_Sym *syms = Tables.sym_tbl;
	char *names = Tables.symstr_tbl;
	int sym_cnt = Tables.sec_header_tbl[Tables.symtbl_ndx].sh_size
		/ Tables.sec_header_tbl[Tables.symtbl_ndx].sh_entsize;
	for(int i = 0; i < sym_cnt; i++) {
		address_t addr = 0;
		symbol_kind_t kind;
		
		// Function symbol
		if(ELF32_ST_TYPE(syms[i].st_info)== STT_FUNC
		&& syms[i].st_shndx != SHN_UNDEF) {
			kind = SYMBOL_Function;
			addr = (address_t)syms[i].st_value;
		}
		
		// Simple label symbol
		else if(ELF32_ST_TYPE(syms[i].st_info)== STT_NOTYPE
		&& syms[i].st_shndx == Text.txt_index) {
			kind = SYMBOL_Label;
			addr = (address_t)syms[i].st_value;
		}

		// Build the label if required
		if(addr) {
			String label(&names[syms[i].st_name]);
			Symbol *sym = new Symbol(file, label, kind, addr);
			file.syms.put(label, sym);
			Inst *inst = (Inst *)findByAddress(addr);
			if(inst)
				inst->set<String>(File::ID_Label, label);
		}
	}
}


/**
 * Find an instructions thanks to its address.
 * @param addr	Address of the instruction to find.
 * @return			Found instruction or null if not found.
 */
otawa::Inst *CodeSegment::findByAddress(address_t addr) {
	
	// In the segement ?
	if(addr < code.address() || addr >= code.address() + code.size())
		return 0;
	
	// Look in the instruction
	/* !!TODO!! May be improved using an indirect table.
	 * Issue: manage the indirect table with modifications of the code.
	 */
	for(otawa::Inst *inst = code.first(); !inst->atEnd(); inst = inst->next())
		if(!inst->isPseudo() && inst->address() == addr)
			return inst;
	return 0;
}


/**
 * @class CodeSegment::Code
 * As, in this loader, there is no function identification, the entire segment is viewed as a monolithic piece of code
 * and only one code representation is required and embeded in the segment object.
 */

// Overloaded
CString CodeSegment::Code::name(void) {
	return "";
}

// Overloaded
address_t CodeSegment::Code::address(void) {
	return addr;
}

// Overloaded
size_t CodeSegment::Code::size(void) {
	return _size;
}


/**
 * Get first instruction of this code item.
 * @return First instruction.
 */
Inst *CodeSegment::Code::first(void) const {
	return (Inst *)insts.first();
}


/**
 * Get the last instruction of this code item.
 * @return Last instruction.
 */
Inst *CodeSegment::Code::last(void) const {
	return (Inst *)insts.last();
}


/**
 * Build a new code item.
 * @param memory	GLISS memory structure.
 * @param address	Segment address.
 * @param size	Segment size.
 */
CodeSegment::Code::Code(memory_t *memory, address_t address, size_t size)
: mem(memory), addr(address), _size(size) {
}


/**
 * Free all instructions.
 */
CodeSegment::Code::~Code(void) {
	while(!insts.isEmpty()) {
		Inst *inst = (Inst *)insts.first();
		insts.removeFirst();
		delete inst;
	}
}

} } // otawa::gliss
