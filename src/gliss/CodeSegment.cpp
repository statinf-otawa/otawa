/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/CodeSegment.cpp -- gliss::CodeSegment class implementation.
 */

#include <elm/debug.h>
#include <otawa/gliss.h>
#include <otawa/gliss/Symbol.h>
#include <otawa/gliss/MemInst.h>

using namespace elm;

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
		Inst *result;
		switch(inst->ident) {

		case ID_LFDUX_FR_R_R: case ID_LFDU_FR_R_: case ID_LFDX_FR_R_R:
		case ID_LFD_FR_R_: case ID_LFSUX_FR_R_R: case ID_LFSU_FR_R_:
		case ID_LFSX_FR_R_R: case ID_LFS_FR_R_: 	case ID_LBZUX_R_R_R:
		case ID_LBZU_R_R_: case ID_LBZX_R_R_R: case ID_LBZ_R_R_:
		case ID_LHAUX_R_R_R: case ID_LHAU_R_R_: case ID_LHAX_R_R_R:
		case ID_LHA_R_R_: case ID_LHZUX_R_R_R: case ID_LHZU_R_R_:
		case ID_LHZX_R_R_R: case ID_LHZ_R_R_: case ID_LWZUX_R_R_R:
		case ID_LWZU_R_R_: case ID_LWZX_R_R_R: case ID_LWZ_R_R_:
		case ID_LWBRX_R_R_R: case ID_LHBRX_R_R_R: case ID_LMW_R_R_:
		case ID_LSWX_R_R_R: case ID_LSWI_R_R_:
		case ID_STFDUX_FR_R_R: case ID_STFDU_FR_R_: case ID_STFDX_FR_R_R:
		case ID_STFD_FR_R_: case ID_STFSUX_FR_R_R: case ID_STFSU_FR_R_:
		case ID_STFSX_FR_R_R: case ID_STFS_FR_R_: case ID_STBUX_R_R_R:
		case ID_STBU_R_R_: case ID_STBX_R_R_R: case ID_STB_R_R_:
		case ID_STHUX_R_R_R: case ID_STHU_R_R_: case ID_STHX_R_R_R:
		case ID_STH_R_R_: case ID_STWUX_R_R_R: case ID_STWU_R_R_:
		case ID_STWX_R_R_R: case ID_STW_R_R_: case ID_STWBRX_R_R_R:
		case ID_STHBRX_R_R_R: case ID_STMW_R_R_: case ID_STSWX_R_R_R:
		case ID_STSWI_R_R_:
			result = new MemInst(*this, addr);
			break;

		case ID_BL_:
			if(inst->instrinput[0].val.Int24 == 1) {
				result = new Inst(*this, addr);
				break;
			}

		case ID_B_: case ID_BA_: case ID_BLA_: case ID_BC_:
		case ID_BCA_: case ID_BCL_: case ID_BCLA_: case ID_BCCTR_:
		case ID_BCCTRL_: case ID_BCLR_: case ID_BCLRL_: case ID_SC:
			result = new ControlInst(*this, addr);
			break;

		default:
			result = new Inst(*this, addr);
			break;
		}
		
		// Cleanup
		code._insts.addLast(result);
		iss_free(inst);
	}

		
	// Add symbols
	address_t lbound = address(), ubound = lbound + size();
	for(Iterator<otawa::Symbol *> sym(file.symbols().visit()); sym; sym++) {
		address_t addr = sym->address();
		if(addr >= lbound && addr < ubound) {
			Inst *inst = (Inst *)findByAddress(addr);
			if(inst) {
				Identifier *id;
				switch(sym->kind()) {
				case SYMBOL_Function:
					inst->add<String>(File::ID_FunctionLabel, sym->name());
				case SYMBOL_Label:
					inst->add<String>( File::ID_Label, sym->name());
					break;
				}
			}
		} 
	}
}


/**
 * Find an instructions thanks to its address.
 * @param addr	Address of the instruction to find.
 * @return			Found instruction or null if not found.
 */
otawa::Inst *CodeSegment::findByAddress(address_t addr) {

	// Already built ?
	if(!built)
		build();
	
	// In the segment ?
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
	return (Inst *)_insts.first();
}


/**
 * Get the last instruction of this code item.
 * @return Last instruction.
 */
Inst *CodeSegment::Code::last(void) const {
	return (Inst *)_insts.last();
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
	while(!_insts.isEmpty()) {
		Inst *inst = (Inst *)_insts.first();
		_insts.removeFirst();
		delete inst;
	}
}

// Internal class
class InstIter: public IteratorInst<otawa::Inst *> {
	inhstruct::DLList& list;
	Inst *cur;
public:
	inline InstIter(inhstruct::DLList& _list): list(_list),
		cur((Inst *)_list.first()) { };
	virtual bool ended(void) const { return cur->atEnd(); };
	virtual Inst *item(void) const { return cur; };
	virtual void next(void) { cur = (Inst *)cur->next(); };
};

// Code overload
IteratorInst<otawa::Inst *> *CodeSegment::Code::insts(void) {
	return new InstIter(_insts);
}


} } // otawa::gliss
