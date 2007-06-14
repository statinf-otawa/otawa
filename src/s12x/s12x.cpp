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
#include <otawa/base.h>
#define ISS_DISASM
#include "emul.h"

#define TRACE(m) //cout << m << io::endl
#define SIZE (inst->size / 8)

namespace otawa { namespace s12x {

static const unsigned long PPAGE_LOW = 0x8000;
static const unsigned long PPAGE_SIZE = 0x4000;
static const unsigned long PPAGE_UP = PPAGE_LOW + PPAGE_SIZE;
static const unsigned long ADDRESS_TOP = 0x10000;

/**
 * This function converts a opr16a + PPAGE value into a unique linear 4Mb address
 * space according the formula below:
 * 	if opr16a < 0x8000 or opr16a >= 0xc000 then opr16a
 *	else 0x10000 + PPAGE x (opr16a - 0x8000).
 */
static address_t toLinear(int opr16a, int ppage) {
	if(opr16a < PPAGE_LOW || opr16a >= PPAGE_UP)
		return opr16a;
	else
		return ADDRESS_TOP + (opr16a - PPAGE_LOW) * ppage;
}


/**
 * Normalize an address to be :
 * @li < 0x10000 (size of the main address space),
 * @li in page 0 if the address is out of [0x8000, 0xc000].
 * @param addr	Address to normalize.
 * @return		Normalized address.
 */
static address_t normalize(address_t addr) {
	if(addr.address() >= ADDRESS_TOP)
		return addr.address() & (ADDRESS_TOP - 1); 
	else if(addr.address() < PPAGE_LOW || addr.address() >= PPAGE_UP)
		return address_t(0, addr);
	else
		return addr;
}


/**
 * Fix a computed address according the instruction computing it.
 * @param inst	Address of the instruction.
 * @param addr	Rough address to fix.
 * @return		Fixed address.
 */ 
static address_t fix(address_t inst, size_t addr) {
	if(addr >= PPAGE_LOW && addr < PPAGE_UP)
		return address_t(inst.page(), addr);
	else
		return addr;
}
 

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
		: otawa::loader::old_gliss::BranchInst(process, kind, addr), _size(0) { }
		
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
int Process::computeSize(Address addr) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(addr.address(), buffer);
	inst = iss_decode((state_t *)state(), addr.address(), buffer);

	// Build the OTAWA instruction
	TRACE("Process::computeSize(" << addr << ") = " << inst->size);
	return SIZE;
}


/**
 */
otawa::Inst *Process::decode(address_t addr) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr.address(), buffer);
	inst = iss_decode((state_t *)state(), (::address_t)addr.address(), buffer);
	TRACE("otawa::s12x::Process::decode(" << addr << ") = " << inst->ident << ", " << inst->size);

	// Build the OTAWA instruction
	if(inst->ident == ID_Instrunknown)
		throw Exception(_ << "unknown instruction at " << addr);
	Inst::kind_t kind = iss_table[inst->ident].otawa_kind;
	if(kind & Inst::IS_CONTROL)
		return new BranchInst(*this, kind, addr);
	else
		return new Inst(*this, kind, addr);	
}


/**
 */
address_t BranchInst::decodeTargetAddress(void) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(address().address(), buffer);
	inst = iss_decode((state_t *)process().state(), address().address(), buffer);

	// Scan instruction
	address_t result;
	switch(inst->ident) {
	
	/* short branches (PC + 2 + s8)
	 *		B{RA|RN}
	 * 		B{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT} */ 
	case ID_BRA_: case ID_BVS_: case ID_BVC_: case ID_BPL_: case ID_BNE_:
	case ID_BMI_: case ID_BEQ_: case ID_BCS_: case ID_BCC_: case ID_BLS_:
    case ID_BHI_: case ID_BLT_: case ID_BLE_: case ID_BGT_: case ID_BGE_:
    	ASSERT(inst->instrinput[0].type == PARAM_INT8_T);
    	result = normalize(address() + SIZE + inst->instrinput[0].val.int8);
    	break;

	/* long branches (PC + (s8)
	 * 		LB{RA, RN}
	 * 		LB{CC|CS|EQ|MI|NE|PL|VC|VS|HI|HS|LO|LS|GE|GT|LE|LT} */
	case ID_LBRA_: case ID_LBVS_: case ID_LBVC_: case ID_LBPL_:
	case ID_LBNE_: case ID_LBMI_: case ID_LBEQ_: case ID_LBCS_:
	case ID_LBCC_: case ID_LBLS_: case ID_LBHI_: case ID_LBLT_:
	case ID_LBLE_: case ID_LBGT_: case ID_LBGE_:
    	ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
    	result = normalize(address() + SIZE + inst->instrinput[0].val.int16);
    	break;

	/* conditional branch on bit test
	 * 		BR{CLR|SET} */
	case ID_BRCLR_: case ID_BRCLR__0:
	case ID_BRCLR_X__0: case ID_BRCLR_Y__0: case ID_BRCLR_SP__0: case ID_BRCLR_PC__0: 
	case ID_BRCLR_X__1: case ID_BRCLR_Y__1: case ID_BRCLR_SP__1: case ID_BRCLR_PC__1: 
	case ID_BRSET_: case ID_BRSET__0:
	case ID_BRSET_X_: case ID_BRSET_Y_: case ID_BRSET_SP_: case ID_BRSET_PC_:
	case ID_BRSET_X__0: case ID_BRSET_Y__0: case ID_BRSET_SP__0: case ID_BRSET_PC__0:
	case ID_BRSET_X__1: case ID_BRSET_Y__1: case ID_BRSET_SP__1: case ID_BRSET_PC__1:	
		ASSERT(inst->instrinput[2].type == PARAM_INT8_T);
		result = normalize(address() + SIZE + inst->instrinput[2].val.int8);
		break;

	case ID_BRCLR_X_: case ID_BRCLR_Y_: case ID_BRCLR_SP_: case ID_BRCLR_PC_: 
	case ID_BRCLR_D_X_: case ID_BRCLR_D_Y_: case ID_BRCLR_D_SP_: case ID_BRCLR_D_PC_:
	case ID_BRCLR_B_X_: case ID_BRCLR_B_Y_: case ID_BRCLR_B_SP_: case ID_BRCLR_B_PC_:
	case ID_BRCLR_A_X_: case ID_BRCLR_A_Y_: case ID_BRCLR_A_SP_: case ID_BRCLR_A_PC_:
	case ID_BRSET_D_X_: case ID_BRSET_D_Y_: case ID_BRSET_D_SP_: case ID_BRSET_D_PC_:
	case ID_BRSET_B_X_: case ID_BRSET_B_Y_: case ID_BRSET_B_SP_: case ID_BRSET_B_PC_:
	case ID_BRSET_A_X_: case ID_BRSET_A_Y_: case ID_BRSET_A_SP_: case ID_BRSET_A_PC_:
		ASSERT(inst->instrinput[1].type == PARAM_INT8_T);
		result = normalize(address() + SIZE + inst->instrinput[1].val.int8);
		break;
 
	 /* loop (PC + 3 + s8)
	  * 		{DB|IB|TB}{EQ|NE} */
	case ID_DBEQ_A_: case ID_DBEQ_B_: case ID_DBEQ_D_: case ID_DBEQ_X_:
	case ID_DBEQ_Y_: case ID_DBEQ_SP_:
	case ID_DBNE_A_:case ID_DBNE_B_: case ID_DBNE_D_: case ID_DBNE_X_:
	case ID_DBNE_Y_: case ID_DBNE_SP_:
	case ID_TBNE_A_: case ID_TBNE_B_: case ID_TBNE_D_: case ID_TBNE_X_:
	case ID_TBNE_Y_: case ID_TBNE_SP_:
	case ID_TBEQ_A_: case ID_TBEQ_B_: case ID_TBEQ_D_: case ID_TBEQ_X_:
	case ID_TBEQ_Y_: case ID_TBEQ_SP_:
	case ID_IBNE_A_: case ID_IBNE_B_: case ID_IBNE_D_: case ID_IBNE_X_:
	case ID_IBNE_Y_: case ID_IBNE_SP_:
	case ID_IBEQ_A_: case ID_IBEQ_B_: case ID_IBEQ_D_: case ID_IBEQ_X_:
	case ID_IBEQ_Y_: case ID_IBEQ_SP_:
    	ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
    	result = normalize(address() + size_t(SIZE - inst->instrinput[0].val.uint8));
    	break;
    	
	case ID_DBEQ_A__0: case ID_DBEQ_B__0: case ID_DBEQ_D__0: case ID_DBEQ_X__0:
	case ID_DBEQ_Y__0: case ID_DBEQ_SP__0:
	case ID_DBNE_A__0: case ID_DBNE_B__0: case ID_DBNE_D__0: case ID_DBNE_X__0:
	case ID_DBNE_Y__0: case ID_DBNE_SP__0:
	case ID_TBNE_A__0: case ID_TBNE_B__0: case ID_TBNE_D__0: case ID_TBNE_X__0:
	case ID_TBNE_Y__0: case ID_TBNE_SP__0:
	case ID_TBEQ_A__0: case ID_TBEQ_B__0: case ID_TBEQ_D__0: case ID_TBEQ_X__0:
	case ID_TBEQ_Y__0: case ID_TBEQ_SP__0:
	case ID_IBNE_A__0: case ID_IBNE_B__0: case ID_IBNE_D__0: case ID_IBNE_X__0:
	case ID_IBNE_Y__0: case ID_IBNE_SP__0:
	case ID_IBEQ_A__0: case ID_IBEQ_B__0: case ID_IBEQ_D__0: case ID_IBEQ_X__0:
	case ID_IBEQ_Y__0: case ID_IBEQ_SP__0:
    	ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
    	result = normalize(address() + SIZE + inst->instrinput[0].val.uint8);
    	break;
	
	/* case ID_DBNE_A__1: case ID_DBNE_B__1: case ID_DBNE_D__1: case ID_DBNE_X__1:
	case ID_DBNE_Y__1: case ID_DBNE_SP__1: */

	/* near jump and calls
	 * 		BSR, JSR (u16)
	 * 		RTS */
	case ID_BSR_:
		ASSERT(inst->instrinput[0].type == PARAM_INT8_T);
		result = normalize(address() + SIZE + inst->instrinput[0].val.int8);
		break;
	case ID_JSR_:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		result = fix(address(), inst->instrinput[0].val.uint8);
		break;
	case ID_JSR__0:
		ASSERT(inst->instrinput[0].type == PARAM_UINT16_T);
		result = fix(address(), inst->instrinput[0].val.uint16);
		break;
    case ID_JSR_PC:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		result = normalize(address() + inst->instrinput[0].val.uint8);
		break;
    case ID_JSR_PC_0: 
	case ID_JSR_PC_2:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		result = normalize(address() - int(inst->instrinput[0].val.uint8));
		break;
	case ID_JSR_PC_1:
		ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
		result = normalize(address() + inst->instrinput[0].val.int16);
		break;
	case ID_JSR_SP: case ID_JSR_Y: case ID_JSR_X: case ID_JSR_SP_0:
    case ID_JSR_Y_0: case ID_JSR_X_0: case ID_JSR_X_1: case ID_JSR_Y_1:
    case ID_JSR_SP_1: case ID_JSR_X_2: case ID_JSR_Y_2: case ID_JSR_SP_2:
    case ID_JSR_X_: case ID_JSR_Y_: case ID_JSR_SP_: case ID_JSR_X__0:
    case ID_JSR_Y__0: case ID_JSR_SP__0: case ID_JSR_SP_3: case ID_JSR_Y_3:
    case ID_JSR_X_3: case ID_JSR_X_4: case ID_JSR_Y_4: case ID_JSR_SP_4:
    case ID_JSR_X_5: case ID_JSR_Y_5: case ID_JSR_SP_5: case ID_JSR_PC_3:
    case ID_JSR_M_D_PC_: case ID_JSR_M_D_SP_: case ID_JSR_M_D_Y_:
    case ID_JSR_M_D_X_: case ID_JSR_M_PC_: case ID_JSR_M_SP_: case ID_JSR_M_Y_:
    case ID_JSR_M_X_: case ID_JSR_A_PC: case ID_JSR_A_SP: case ID_JSR_A_Y:
    case ID_JSR_A_X: case ID_JSR_B_PC: case ID_JSR_B_SP: case ID_JSR_B_Y:
    case ID_JSR_B_X: case ID_JSR_D_PC: case ID_JSR_D_SP: case ID_JSR_D_Y:
    case ID_JSR_D_X:
		result = 0;
		break;
	case ID_RTS:
		result = 0;
		break;	
		
	/* far calls
	 *		CALL (expanded memory)
	 *		RTC
	 */
	case ID_CALL_:
		ASSERT(inst->instrinput[0].type == PARAM_UINT16_T);
		ASSERT(inst->instrinput[1].type == PARAM_UINT8_T);
		result = toLinear(
			inst->instrinput[0].val.uint16,
			inst->instrinput[1].val.uint8);
		break;
	case ID_CALL_PC_:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		ASSERT(inst->instrinput[1].type == PARAM_UINT8_T);
		result = toLinear(
			address() + inst->instrinput[0].val.uint8,
			inst->instrinput[1].val.uint8);
		break;
    case ID_CALL_PC__1:
		ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
		ASSERT(inst->instrinput[1].type == PARAM_UINT8_T);
		result = toLinear(
			address() + inst->instrinput[0].val.int16,
			inst->instrinput[1].val.uint8);
		break;
    case ID_CALL_PC__2: case ID_CALL_PC__3:
    case ID_CALL_D_X_: case ID_CALL_D_Y_: case ID_CALL_D_SP_:  case ID_CALL_D_PC_:
    case ID_CALL_B_X_: case ID_CALL_B_Y_: case ID_CALL_B_SP_: case ID_CALL_B_PC_:
    case ID_CALL_A_X_: case ID_CALL_A_Y_: case ID_CALL_A_SP_: case ID_CALL_A_PC_:
    case ID_CALL_M_X_: case ID_CALL_M_Y_: case ID_CALL_M_SP_: case ID_CALL_M_PC_:
    case ID_CALL_M_D_X_: case ID_CALL_M_D_Y_: case ID_CALL_M_D_SP_: case ID_CALL_M_D_PC_:
    case ID_CALL_SP_: case ID_CALL_Y_: case ID_CALL_X_: case ID_CALL_SP__0:
    case ID_CALL_Y__0: case ID_CALL_X__0: case ID_CALL_X__1: case ID_CALL_Y__1:
    case ID_CALL_SP__1: case ID_CALL_SP__2: case ID_CALL_Y__2: case ID_CALL_X__2:
    case ID_CALL_SP__3: case ID_CALL_Y__3: case ID_CALL_X__3: case ID_CALL_SP__4:
    case ID_CALL_Y__4: case ID_CALL_X__4: case ID_CALL_SP__5: case ID_CALL_Y__5:
    case ID_CALL_X__5: case ID_CALL_X__6: case ID_CALL_Y__6: case ID_CALL_SP__6:
    case ID_CALL_X__7: case ID_CALL_Y__7: case ID_CALL_SP__7:
	case ID_RTC:
		result = 0;
		break;

	/* jump
	 * 		JMP (u16) */
	case ID_JMP_:
		ASSERT(inst->instrinput[0].type == PARAM_UINT16_T);
		result = fix(address(), inst->instrinput[0].val.uint16);
		break;
	case ID_JMP_PC:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		result = normalize(address() + inst->instrinput[0].val.uint8);
		break;
	case ID_JMP_PC_0:
		ASSERT(inst->instrinput[0].type == PARAM_UINT8_T);
		result = normalize(address() - int(inst->instrinput[0].val.uint8));
		break;
	case ID_JMP_PC_1:
		ASSERT(inst->instrinput[0].type == PARAM_INT16_T);
		result = normalize(address() + inst->instrinput[0].val.int16);
		break;
	case ID_JMP_PC_2: case ID_JMP_PC_3:
	case ID_JMP_SP: case ID_JMP_Y: case ID_JMP_X: case ID_JMP_SP_0:
    case ID_JMP_Y_0: case ID_JMP_X_0: case ID_JMP_X_1: case ID_JMP_Y_1:
	case ID_JMP_SP_1: case ID_JMP_X_2: case ID_JMP_Y_2: case ID_JMP_SP_2:
	case ID_JMP_X_: case ID_JMP_Y_: case ID_JMP_SP_: case ID_JMP_X__0:
	case ID_JMP_Y__0: case ID_JMP_SP__0: case ID_JMP_SP_3: case ID_JMP_Y_3:
	case ID_JMP_X_3: case ID_JMP_X_4: case ID_JMP_Y_4: case ID_JMP_SP_4:
    case ID_JMP_X_5: case ID_JMP_Y_5: case ID_JMP_SP_5: case ID_JMP_M_D_PC_:
	case ID_JMP_M_D_SP_: case ID_JMP_M_D_Y_: case ID_JMP_M_D_X_: case ID_JMP_M_PC_:
	case ID_JMP_M_SP_: case ID_JMP_M_Y_: case ID_JMP_M_X_: case ID_JMP_A_PC:
	case ID_JMP_A_SP: case ID_JMP_A_Y: case ID_JMP_A_X: case ID_JMP_B_PC:
    case ID_JMP_B_SP: case ID_JMP_B_Y: case ID_JMP_B_X: case ID_JMP_D_PC:
    case ID_JMP_D_SP: case ID_JMP_D_Y: case ID_JMP_D_X:
		result = 0;
		break;

	/* exception and system call
	 * 		RTI
	 * 		SWI
	 * 		TRAP (not defined ?) */
	case ID_RTI:
	case ID_SWI:
		result = 0;
		break;	

	default:
		ASSERT(false);
	}
	
	// Result
	TRACE("otawa::s12x::BranchInst::decodeTargetAddress(" << address()
		<< ") = " << result);
	return result;
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
	"elf_53"
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
otawa::s12x::Loader& s12x_plugin = OTAWA_LOADER_HOOK;
