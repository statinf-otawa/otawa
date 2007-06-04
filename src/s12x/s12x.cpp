/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	Star12X plugin implementation
 */

#include <elm/assert.h>
#include <otawa/loader/old_gliss/Process.h>
#include <otawa/loader/old_gliss/BranchInst.h>
#include <otawa/prog/Loader.h>
#include <otawa/hard.h>
#include "emul.h"

#define TRACE(m) cout << m << io::endl

namespace otawa { namespace s12x {

// Platform class
class Platform: public hard::Platform {
public:
	static const Identification ID;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Registers
	static hard::Register A;
	static hard::Register B;
	static hard::Register IX;
	static hard::Register IY;
	static hard::Register SP;
	static hard::Register CCR;
	static const hard::MeltedBank REGS;
};

// Process class
class Process: public otawa::loader::old_gliss::Process {

public:
	Process(Manager *manager, hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 0; }
	int computeSize(address_t addr);

protected:
	virtual otawa::Inst *decode(address_t addr);
};

// Inst class
class Inst: public otawa::loader::old_gliss::Inst {
public:

	inline Inst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::old_gliss::Inst(process, kind, addr), _size(0) { }
		
	virtual size_t size(void) const {
		if(!_size)
			_size = ((Process &)process()).computeSize(address());
		return _size;
	}

private:
	mutable int _size;
};


// BranchInst class
class BranchInst: public otawa::loader::old_gliss::BranchInst {
public:

	inline BranchInst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::old_gliss::BranchInst(process, kind, addr) { }
		
	virtual size_t size(void) const {
		if(!_size)
			_size = ((Process &)process()).computeSize(address());
		return _size;
	}

protected:
	virtual address_t decodeTargetAddress(void);

private:
	mutable int _size;	
};


/**
 * Register banks.
 */
hard::Register Platform::A("A", hard::Register::INT, 8);
hard::Register Platform::B("B", hard::Register::INT, 8);
hard::Register Platform::IX("IX", hard::Register::ADDR, 16);
hard::Register Platform::IY("IY", hard::Register::ADDR, 16);
hard::Register Platform::SP("SP", hard::Register::ADDR, 16);
hard::Register Platform::CCR("CCR", hard::Register::BITS, 16);
const hard::MeltedBank Platform::REGS("REGS",
	&Platform::A,
	&Platform::B,
	&Platform::IX,
	&Platform::IY,
	&Platform::SP,
	&Platform::CCR,
	0);
static const hard::RegBank *banks[] = {
	&Platform::REGS,
};
static const elm::genstruct::Table<const hard::RegBank *> banks_table(banks, 1);


/**
 * Identification of the default platform.
 */
const Platform::Identification Platform::ID("s12x-elf-*");


/**
 * Build a new gliss platform with the given configuration.
 * @param props		Configuration properties.
 */
Platform::Platform(const PropList& props): hard::Platform(ID, props) {
	_banks = &banks_table;
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: hard::Platform(platform, props) {
	_banks = &banks_table;
}


/**
 * Build the process.
 * @param manager	Current manager.
 * @param pf		Running platform.
 * @param props		Configuration properties.
 */
Process::Process(
	Manager *manager,
	hard::Platform *pf,
	const PropList& props
): otawa::loader::old_gliss::Process(manager, pf, props) {
}


/**
 * Compute the size of an instruction.
 * @param addr	Address of the instruction.
 * @return		Instructrion size in bytes.
 */
int Process::computeSize(address_t addr) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode((state_t *)state(), (::address_t)addr, buffer);

	// Build the OTAWA instruction
	return inst->size;
}


/**
 */
otawa::Inst *Process::decode(address_t addr) {
	TRACE("decode(" << addr << ")");

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode((state_t *)state(), (::address_t)addr, buffer);

	// Build the OTAWA instruction
	Inst::kind_t kind = iss_table[inst->ident].otawa_kind;
	if(kind & Inst::IS_CONTROL)
		return new BranchInst(*this, kind, addr);
	else
		return new Inst(*this, kind, addr);	
	// !!TODO!!
	/* Branch instructions
	 * short branches (PC + s8)
	 *		B{RA|RN}
	 * 		B{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT}
	 * 		BR{CLR|SET}
	 * long branches (PC + (s8)
	 * 		LB{RA, RN}
	 * 		LB{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT}
	 * loop (PC + s9)
	 * 		{DB|IB|TB}{EQ|NE}
	 * jump and calls
	 * 		BSR, JSR (u16)
	 *		CALL (expanded memory)
	 * 		JMP (u16)
	 * 		RTC
	 * 		RTS
	 * exception
	 * 		RTI
	 * 		SWI
	 * 		TRAP
	 */
}


/**
 */
address_t BranchInst::decodeTargetAddress(void) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(address(), buffer);
	inst = iss_decode((state_t *)process().state(), address(), buffer);

	// Scan instruction
	switch(inst->ident) {
	
	/* short branches (PC + 2 + s8)
	 *		B{RA|RN}
	 * 		B{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT} */ 
	case ID_BRA_: case ID_BVS_: case ID_BVC_: case ID_BPL_: case ID_BNE_:
	case ID_BMI_: case ID_BEQ_: case ID_BCS_: case ID_BCC_: case ID_BLS_:
    case ID_BHI_: case ID_BLT_: case ID_BLE_: case ID_BGT_: case ID_BGE_:
    	ASSERT(inst->instrinput[0].type == PARAM_INT8_T);
    	return address() + 2 + inst->instrinput[0].val.int8;
		break;
    /*case ID_BRCLR_: case ID_BRCLR_D_X_: case ID_BRCLR_D_Y_: case ID_BRCLR_D_SP_:
    case ID_BRCLR_D_PC_: case ID_BRCLR_B_X_: case ID_BRCLR_B_Y_: case ID_BRCLR_B_SP_:
    case ID_BRCLR_B_PC_: case ID_BRCLR_A_X_: case ID_BRCLR_A_Y_: case ID_BRCLR_A_SP_:
    case ID_BRCLR_A_PC_: case ID_BRCLR_PC_: case ID_BRCLR_SP_: case ID_BRCLR_Y_:
    case ID_BRCLR_X_: case ID_BRCLR_PC__0: case ID_BRCLR_SP__0: case ID_BRCLR_Y__0:
    case ID_BRCLR_X__0: case ID_BRCLR_X__1: case ID_BRCLR_Y__1: case ID_BRCLR_SP__1:
    case ID_BRCLR_PC__1: case ID_BRCLR_SP__2: case ID_BRCLR_Y__2: case ID_BRCLR_X__2:
    case ID_BRCLR_SP__3: case ID_BRCLR_Y__3: case ID_BRCLR_X__3: case ID_BRCLR_SP__4:
    case ID_BRCLR_Y__4: case ID_BRCLR_X__4: case ID_BRCLR_SP__5: case ID_BRCLR_Y__5:
    case ID_BRCLR_X__5: case ID_BRCLR_X__6: case ID_BRCLR_Y__6: case ID_BRCLR_SP__6:
    case ID_BRCLR_PC__2: case ID_BRCLR_X__7: case ID_BRCLR_Y__7: case ID_BRCLR_SP__7:
    case ID_BRCLR_PC__3: case ID_BRCLR__0: case ID_BRSET_: case ID_BRSET_PC_:
    case ID_BRSET_SP_: case ID_BRSET_Y_: case ID_BRSET_X_: case ID_BRSET_X__0:
    case ID_BRSET_Y__0: case ID_BRSET_SP__0: case ID_BRSET_PC__0: case ID_BRSET_X__1:
    case ID_BRSET_Y__1: case ID_BRSET_SP__1: case ID_BRSET_PC__1: case ID_BRSET_A_PC_:
    case ID_BRSET_A_SP_: case ID_BRSET_A_Y_: case ID_BRSET_A_X_: case ID_BRSET_B_PC_:
    case ID_BRSET_B_SP_: case ID_BRSET_B_Y_: case ID_BRSET_B_X_: case ID_BRSET_D_PC_:
    case ID_BRSET_D_SP_: case ID_BRSET_D_Y_: case ID_BRSET_D_X_: case ID_BRSET__0:
    	ASSERT(inst->instrinput[0].type == PARAM_INT8_T);
    	return addr + 2 + inst->instrinput[0].val.int8;
    	break;*/
	 /* 
	 * 		BR{CLR|SET}
	 */
	 
	/* long branches (PC + (s8)
	 * 		LB{RA, RN}
	 * 		LB{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT} */
    case ID_LBRA_: case ID_LBVS_: case ID_LBVC_: case ID_LBPL_:
    case ID_LBNE_: case ID_LBMI_: case ID_LBEQ_: case ID_LBCS_:
    case ID_LBCC_: case ID_LBLS_: case ID_LBHI_: case ID_LBLT_:
    case ID_LBLE_: case ID_LBGT_: case ID_LBGE_:
    	ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
    	return address() + 2 + inst->instrinput[0].val.int16;
		break;
	 
	 /* loop (PC + s9)
	  * 		{DB|IB|TB}{EQ|NE} */
	case ID_DBNE_SP_: case ID_DBNE_SP__0: case ID_DBNE_Y_: case ID_DBNE_Y__0:
    case ID_DBNE_X_: case ID_DBNE_X__0: case ID_DBNE_D_: case ID_DBNE_D__0:
    case ID_DBNE_B_: case ID_DBNE_B__0: case ID_DBNE_A_: case ID_DBNE_A__0:
    case ID_DBNE_A__1: case ID_DBNE_B__1: case ID_DBNE_D__1: case ID_DBNE_X__1:
    case ID_DBNE_Y__1: case ID_DBNE_SP__1:
    case ID_DBEQ_A_: case ID_DBEQ_B_: case ID_DBEQ_D_: case ID_DBEQ_X_:
    case ID_DBEQ_Y_: case ID_DBEQ_SP_: case ID_DBEQ_A__0: case ID_DBEQ_B__0:
    case ID_DBEQ_D__0: case ID_DBEQ_X__0: case ID_DBEQ_Y__0: case ID_DBEQ_SP__0:
    case ID_TBNE_A_: case ID_TBNE_B_: case ID_TBNE_D_: case ID_TBNE_X_:
    case ID_TBNE_Y_: case ID_TBNE_SP_: case ID_TBNE_A__0: case ID_TBNE_B__0:
    case ID_TBNE_D__0: case ID_TBNE_X__0: case ID_TBNE_Y__0: case ID_TBNE_SP__0:
    case ID_TBEQ_A_: case ID_TBEQ_B_: case ID_TBEQ_D_: case ID_TBEQ_X_:
    case ID_TBEQ_Y_: case ID_TBEQ_SP_: case ID_TBEQ_A__0: case ID_TBEQ_B__0:
    case ID_TBEQ_D__0: case ID_TBEQ_X__0: case ID_TBEQ_Y__0: case ID_TBEQ_SP__0:
    case ID_IBNE_A_: case ID_IBNE_B_: case ID_IBNE_D_: case ID_IBNE_X_:
    case ID_IBNE_Y_: case ID_IBNE_SP_: case ID_IBNE_A__0: case ID_IBNE_B__0:
    case ID_IBNE_D__0: case ID_IBNE_X__0: case ID_IBNE_Y__0: case ID_IBNE_SP__0:
    case ID_IBEQ_A_: case ID_IBEQ_B_: case ID_IBEQ_D_: case ID_IBEQ_X_:
    case ID_IBEQ_Y_: case ID_IBEQ_SP_: case ID_IBEQ_A__0: case ID_IBEQ_B__0:
    case ID_IBEQ_D__0: case ID_IBEQ_X__0: case ID_IBEQ_Y__0: case ID_IBEQ_SP__0:
    	ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
    	return address() + 3 + inst->instrinput[0].val.int16;
		break;

	 
	
	/* jump and calls
	 * 		BSR, JSR (u16)
	 *		CALL (expanded memory)
	 * 		JMP (u16)
	 * 		RTC
	 * 		RTS
	 * exception
	 * 		RTI
	 * 		SWI
	 * 		TRAP
	 */
	default:
		ASSERT(false);
	}
}


// otawa::gliss::Loader class
class Loader: public otawa::Loader {
public:
	Loader(void);

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *load(Manager *_man, CString path, const PropList& props);
	virtual otawa::Process *create(Manager *_man, const PropList& props);
};


// Alias table
static CString table[] = {
	"elf_70"
};
static elm::genstruct::Table<CString> s12x_aliases(table, 1);
 

/**
 * Build a new loader.
 */
Loader::Loader(void)
: otawa::Loader("s12x", Version(1, 0, 0), OTAWA_LOADER_VERSION, s12x_aliases) {
}


/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const {
	return "s12x";
}

/**
 * Load a file with the current loader.
 * @param man		Caller manager.
 * @param path		Path to the file.
 * @param props	Properties.
 * @return	Created process or null if there is an error.
 */
otawa::Process *Loader::load(Manager *man, CString path, const PropList& props) {
	otawa::Process *proc = create(man, props);
	if(!proc->loadProgram(path)) {
		delete proc;
		return 0;
	}
	else
		return proc;
}


/**
 * Create an empty process.
 * @param man		Caller manager.
 * @param props	Properties.
 * @return		Created process.
 */
otawa::Process *Loader::create(Manager *man, const PropList& props) {
	return new Process(man, new Platform(props), props);
}

} }	// otawa::s12x


// Star12X GLISS Loader entry point
otawa::s12x::Loader OTAWA_LOADER_HOOK;
otawa::s12x::Loader& arm_plugin = OTAWA_LOADER_HOOK;
