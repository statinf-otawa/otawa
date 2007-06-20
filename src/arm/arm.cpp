/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	ARM plugin implementation
 */

#include <elm/assert.h>
#include <otawa/loader/old_gliss/Process.h>
#include <otawa/loader/old_gliss/BranchInst.h>
#include <otawa/prog/Loader.h>
#include "emul.h"

#define TRACE(m) cout << m << io::endl

namespace otawa { namespace arm {

// Process class
class Process: public otawa::loader::old_gliss::Process {

public:
	Process(Manager *manager, hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 4; }

protected:
	virtual otawa::Inst *decode(address_t addr);
};

// Inst class
class Inst: public otawa::loader::old_gliss::Inst {
public:

	inline Inst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::old_gliss::Inst(process, kind, addr) { }
		
	virtual size_t size(void) const { return 4; }
};


// BranchInst class
class BranchInst: public otawa::loader::old_gliss::BranchInst {
public:

	inline BranchInst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::old_gliss::BranchInst(process, kind, addr) { }
		
	virtual size_t size(void) const { return 4; }
	
protected:
	virtual address_t decodeTargetAddress(void);
};


/**
 */
otawa::Inst *Process::decode(address_t addr) {
	TRACE("decode(" << addr << ")");

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode((state_t *)state(), (::address_t)addr, buffer, 0);

	// Look condition	
	Inst::kind_t kind = 0;
	if(inst->instrinput[0].val.uint8 != 14)
		kind |= Inst::IS_COND;

	// Look the instruction
	switch(inst->ident) {

	case ID_Instrunknown:
		return new Inst(*this, 0, addr);

	 case ID_MOV__1:
		if(inst->instrinput[2].val.uint8 == 15
		&& inst->instrinput[5].val.uint8 == 14
		&& inst->instrinput[3].val.uint8 == 0) {
			kind = Inst::IS_CONTROL | Inst::IS_RETURN;
			goto branch;
		}
	
	case ID_UMLAL_: case ID_SMLAL_:
	case ID_UMULL_: case ID_SMULL_:
    case ID_MUL_: case ID_MLA_:
    case ID_TST_: case ID_TST__0: case ID_TST__1:
    case ID_TEQ_: case ID_TEQ__0: case ID_TEQ__1:
    case ID_SUB_: case ID_SUB__0: case ID_SUB__1:
    case ID_SBC_: case ID_SBC__0: case ID_SBC__1:
    case ID_RSC_: case ID_RSC__0: case ID_RSC__1:
    case ID_RSB_: case ID_RSB__0: case ID_RSB__1:
    case ID_ORR_: case ID_ORR__0: case ID_ORR__1:
    case ID_MVN_: case ID_MVN__0: case ID_MVN__1:
    case ID_MOV_: case ID_MOV__0:     
    case ID_EOR_: case ID_EOR__0: case ID_EOR__1:
    case ID_CMP_: case ID_CMP__0: case ID_CMP__1:
    case ID_CMN_: case ID_CMN__0: case ID_CMN__1:     
    case ID_BIC_: case ID_BIC__0: case ID_BIC__1:
    case ID_AND_: case ID_AND__0: case ID_AND__1:
    case ID_ADD_: case ID_ADD__0: case ID_ADD__1:
    case ID_ADC_: case ID_ADC__0: case ID_ADC__1:
    case ID_CLZ_:
    case ID_SWPB_: case ID_SWP_:
		kind |= Inst::IS_INT | Inst::IS_ALU;
		goto simple;

    case ID_BX_:
		kind |= Inst::IS_CONTROL;
		goto branch;

    case ID_B_:
		kind |= Inst::IS_CONTROL;
		if(inst->instrinput[1].val.uint8)
			kind |= Inst::IS_CALL;
		goto branch;
    
    case ID_LDRSH_: case ID_LDRSH__0: case ID_LDRSB_: case ID_LDRSB__0:
    case ID_LDRH_: case ID_LDRH__0:
		kind |= Inst::IS_INT | Inst::IS_LOAD | Inst::IS_MEM;
		goto simple;
			
    case ID_STRH_: case ID_STRH__0:
		kind |= Inst::IS_INT | Inst::IS_STORE | Inst::IS_MEM;
		goto simple;

    case ID_R_: case ID_R__0:
		kind |= Inst::IS_INT | Inst::IS_MEM;
		if(inst->instrinput[5].val.uint8)
			kind |= Inst::IS_LOAD;
		else
			kind |= Inst::IS_STORE;
		goto simple;
    
    case ID_MRS_SPSR: case ID_MRS_CPSR: case ID_MSR_CPSR_F_:
    case ID_MSR_CPSR_S_: case ID_MSR_CPSR_X_: case ID_MSR_CPSR_C_:
    case ID_MSR_CPSR_FS_: case ID_MSR_CPSR_FX_: case ID_MSR_CPSR_F__0:
    case ID_MSR_CPSR_SX_: case ID_MSR_CPSR_SC_: case ID_MSR_CPSR_XC_:
    case ID_MSR_CPSR_FSX_: case ID_MSR_CPSR_FXC_: case ID_MSR_CPSR_SXC_:
    case ID_MSR_CPSR_FSXC_: case ID_MSR_CPSR_: case ID_MSR_SPSR_F_:
    case ID_MSR_SPSR_S_: case ID_MSR_SPSR_X_: case ID_MSR_SPSR_C_:
    case ID_MSR_SPSR_FS_: case ID_MSR_SPSR_FX_: case ID_MSR_SPSR_F__0:
    case ID_MSR_SPSR_SX_: case ID_MSR_SPSR_SC_: case ID_MSR_SPSR_XC_:
    case ID_MSR_SPSR_FSX_: case ID_MSR_SPSR_FXC_: case ID_MSR_SPSR_SXC_:
    case ID_MSR_SPSR_FSXC_: case ID_MSR_SPSR_: case ID_MSR_CPSR_F__1:
    case ID_MSR_CPSR_S__0: case ID_MSR_CPSR_X__0: case ID_MSR_CPSR_C__0:
    case ID_MSR_CPSR_FS__0: case ID_MSR_CPSR_FX__0: case ID_MSR_CPSR_F__2:
    case ID_MSR_CPSR_SX__0: case ID_MSR_CPSR_SC__0: case ID_MSR_CPSR_XC__0:
    case ID_MSR_CPSR_FSX__0: case ID_MSR_CPSR_FXC__0: case ID_MSR_CPSR_SXC__0:
    case ID_MSR_CPSR_FSXC__0: case ID_MSR_CPSR__0: case ID_MSR_SPSR_F__1:
    case ID_MSR_SPSR_S__0: case ID_MSR_SPSR_X__0: case ID_MSR_SPSR_C__0:
    case ID_MSR_SPSR_FS__0: case ID_MSR_SPSR_FX__0: case ID_MSR_SPSR_F__2:
    case ID_MSR_SPSR_SX__0: case ID_MSR_SPSR_SC__0: case ID_MSR_SPSR_XC__0:
    case ID_MSR_SPSR_FSX__0: case ID_MSR_SPSR_FXC__0: case ID_MSR_SPSR_SXC__0:
    case ID_MSR_SPSR_FSXC__0: case ID_MSR_SPSR__0:
		kind |= Inst::IS_INTERN;
		goto simple;    
        
    case ID_M_:
		kind |= Inst::IS_INT | Inst::IS_MEM;
		if(inst->instrinput[4].val.uint8)
			kind |= Inst::IS_LOAD;
		else
			kind |= Inst::IS_STORE;
		goto simple;    
    
    case ID_SWI_:
    	kind |= Inst::IS_TRAP;
    	goto branch;
 
    branch:
		return new BranchInst(*this, kind, addr);
    
    simple:
		return new Inst(*this, kind, addr);
		
	default:
		ASSERT(false);
	}
}


/**
 */
address_t BranchInst::decodeTargetAddress(void) {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(address(), buffer);
	inst = iss_decode((state_t *)process().state(), address(), buffer, 0);

	// San instruction
	switch(inst->ident) {
    case ID_BX_:
    case ID_SWI_:
    case ID_MOV__1:
    	return 0;
    case ID_B_: {
    	return inst->instrinput[2].val.int32 << 2;
    }	
	default:
		ASSERT(false);
	}
}


// Platform definition
static hard::Platform::Identification PFID("arm-*-*");


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
	"elf_40"
};
static elm::genstruct::Table<CString> arm_aliases(table, 1);
 

/**
 * Build a new loader.
 */
Loader::Loader(void)
: otawa::Loader("arm", Version(1, 0, 0), OTAWA_LOADER_VERSION, arm_aliases) {
}


/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const {
	return "arm";
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
	return new Process(man, new hard::Platform(PFID, props), props);
}

} }	// otawa::arm


// ARM GLISS Loader entry point
otawa::arm::Loader OTAWA_LOADER_HOOK;
otawa::arm::Loader& arm_plugin = OTAWA_LOADER_HOOK;

namespace otawa { namespace arm {
	
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
): otawa::loader::old_gliss::Process(manager, &arm_plugin, pf, props) {
}

} } // otawa::arm
