/*
 *	$Id$
 *	ARM plugin implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/assert.h>
#define ISS_DISASM
#include "emul.h"
#include <otawa/loader/old_gliss/Process.h>
#include <otawa/loader/old_gliss/BranchInst.h>
#include <otawa/prog/Loader.h>

#define TRACE(m) //cout << m << io::endl


/*
 * FLOW DECODING
 *	- branches : B label, Bcnd label
 *  - call : BL label
 *  - return :	LDMIA sp, {..., pc, ..., sp, ...}
 *  			MOV pc, lr
 * 				BX lr
 *  - shortcut return : B label (GCC -O2)
 *  - guarded statement : op+cnd (GGC -O2) (special pass in decoding)
 * 
 *  - indirect call (normal and -O2)
 * 		mov lr, pc
 * 		mov pc, ri
 * 		NOTE: function address are stored in data format in the code
 *		ldr	ri, [pc, #offset]
 * 		function address at (instruction address + 8 + offset)
 * 
 *  - switch indirect branch (normale and -O2)
 * 		cmp		ri, #table_size
 * 		ldrls	pc, [pc, ri, lsl #2]	@ indirect branch
 *		b		default_label
 * 		<indirect table>
 */

namespace otawa { namespace arm {

// Specialized registers
static const int pc = 15;
static const int lr = 14;
static const int sp = 13;


/**
 * Test if the instruction read the given register.
 * @param inst	Instruction to look in.
 * @param reg	Register to check.
 * @return		True if the register is read, false else.
 */
static bool readsReg(instruction_t *inst, int reg) {
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(inst->instrinput[i].type == GPR_T
		&& inst->instrinput[i].val.uint8 == reg)
			return true;
	return false;
}


/**
 * Test if the instruction write the given register.
 * @param inst	Instruction to look in.
 * @param reg	Register to check.
 * @return		True if the register is written, false else.
 */
static bool writesReg(instruction_t *inst, int reg) {
	for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(inst->instroutput[i].type == GPR_T
		&& inst->instroutput[i].val.uint8 == reg)
			return true;
	return false;
}


/**
 * Test if the instruction is conditional (guarded).
 * @param inst	Instruction to test.
 * @return		True if it is conditional, false else.
 */
static bool isConditional(instruction_t *inst) {
	return inst->instrinput[0].val.uint8 != 14;
}


// Process class
class Process: public otawa::loader::old_gliss::Process {

public:
	Process(Manager *manager, hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 4; }

protected:
	virtual otawa::Inst *decode(address_t addr);
	virtual void *memory(void) { return ((state_t *)state())->M; }
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
	//char out_buffer[200];
	instruction_t *inst;
	iss_fetch(addr.offset(), buffer);
	inst = iss_decode((state_t *)state(), addr.offset(), buffer);

	// Look condition	
	Inst::kind_t kind = 0;
	if(isConditional(inst))
		kind |= Inst::IS_COND;

	// Look the instruction
	switch(inst->ident) {

	case ID_Instrunknown:
		//cerr << addr << ": unknown\n";
		return new Inst(*this, 0, addr);

	 case ID_MOV__1:
		 if(inst->instrinput[2].val.uint8 == pc
		 && inst->instrinput[5].val.uint8 == lr
		 && inst->instrinput[3].val.uint8 == 0) {
			 kind |= Inst::IS_RETURN;
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
    	if(inst->instrinput[1].val.uint8 == lr)
    		kind |= Inst::IS_RETURN;
    	goto branch;

    case ID_B_:
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
		if(inst->instrinput[5].val.uint8) {
			kind |= Inst::IS_LOAD;
			if(inst->instrinput[7].val.uint8 != pc)
				return new Inst(*this, kind, addr);
			else
				goto branch;
		}
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
    	/*for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
    		if(inst->instroutput[i].type == GPR_T)
    			cerr << addr << ": written to r" << inst->instroutput[i].val.uint8 << io::endl;*/
		kind |= Inst::IS_INT | Inst::IS_MEM;
		if(inst->instrinput[4].val.uint8) {
			/*for(int i = 7; i <= 10; i++)
				cerr << addr << ": M[" << i << "] = " << io::hex(inst->instrinput[i].val.uint8) << io::endl;*/
			kind |= Inst::IS_LOAD;
			if(inst->instrinput[5].val.uint8 == sp
			&& /*writesReg(inst, pc)*/ (inst->instrinput[7].val.uint8 && (1 << (pc - 12)))) {
					kind |= Inst::IS_RETURN;
					goto branch;
				}
		}
		else
			kind |= Inst::IS_STORE;
		goto simple;    
    
    case ID_SWI_:
    	kind |= Inst::IS_TRAP;
    	goto branch;
 
	// Check if the PC is modified
    simple:
    	if(!writesReg(inst, pc)) {
        	//cerr << addr << ": no branch\n";
    		return new Inst(*this, kind, addr);
    	}
    	/*for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
    		if(inst->instroutput[i].type == GPR_T)
    			cerr << addr << ": written to r" << inst->instroutput[i].val.uint8 << io::endl;*/

		
	// Create just a branch instruction 
	branch:
		kind |= Inst::IS_CONTROL;
		/*cerr << addr << ": ";
		if(kind & Inst::IS_RETURN)
			cerr << "return";
		else if(kind & Inst::IS_CALL)
			cerr << "call";
		else
			cerr << "branch found";
		if(kind & Inst::IS_COND)
			cerr << " conditional";
		cerr << io::endl;*/
		return new BranchInst(*this, kind, addr);	    
	}
	ASSERTP(false, (_ << "id " << io::hex(inst->ident) << "not handled at " << addr));
	return 0;
}


/**
 */
address_t BranchInst::decodeTargetAddress(void) {

	// Decode the instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch(address().offset(), buffer);
	inst = iss_decode((state_t *)process().state(), address().offset(), buffer);

	// San instruction
	switch(inst->ident) {
    case ID_B_: {
    		long off = inst->instrinput[2].val.int32;
    		if(off >= 0x2000000)
    			off = (off << 6) >> 6;
    		Address to = address() + int(off + 8); 
    		//cerr << address() << ": branch to " << to << "(" << io::hex(off) << ")" << io::endl; 
    		return to;
    	}
    case ID_BX_:
    case ID_MOV__1:
    case ID_R__0:
    case ID_SWI_:
    case ID_M_:
    	return 0;
	default:
		//ASSERTP(false, "internal error at " << address());
		return 0;
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
	provide(CONTROL_DECODING_FEATURE);
}

} } // otawa::arm
