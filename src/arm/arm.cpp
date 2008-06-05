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
#include <otawa/hard/Register.h>
#include <otawa/loader/arm.h>

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

using namespace otawa::hard;

namespace otawa { namespace arm {


// Specialized registers
static const int pc = 15;
static const int lr = 14;
static const int sp = 13;

// Registers
static PlainBank gpr("GPR", Register::INT, 32, "r%d", 16);
static Register sr("sr", Register::BITS, 32);
static MeltedBank misc("misc", &sr, 0);
static const RegBank *banks_tab[] = { &gpr, &misc };
static Table<const RegBank *> banks(banks_tab, 2);


/**
 * Count the number of ones in bit quadruplet.
 * @param quad	Quadruplet to count in.
 * @return		Number of ones. 
 */
static int countQuad(int quad) {
	int cnt = 0;
	for(int i = 0; i < 4; i++) {
		if(quad & 0x1)
			cnt++;
		quad >>= 1;
	}
	return cnt;
}


/**
 * Record the register in bit quadruplet.
 * @param quad		Quadruplet to look in.
 * @param offset	Offset in the GPR bank.
 * @param reg		To record in.
 * @param cnt		Count in reg.
 */
static void recordQuad(
	int quad,
	int offset,
	elm::genstruct::AllocatedTable<hard::Register *>& reg,
	int& cnt)
{
	for(int i = 0; i < 4; i++) {
		if(quad & 0x1)
			reg[cnt++] = gpr[offset + i];
		quad >>= 1;
	}	
}


/**
 * Test if the instruction read the given register.
 * @param inst	Instruction to look in.
 * @param reg	Register to check.
 * @return		True if the register is read, false else.
 */
/*static bool readsReg(instruction_t *inst, int reg) {
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(inst->instrinput[i].type == GPR_T
		&& inst->instrinput[i].val.uint8 == reg)
			return true;
	return false;
}*/


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
 * Test if the instruction is conditional (guarded) (argument 0).
 * @param inst	Instruction to test.
 * @return		True if it is conditional, false else.
 */
static bool isConditional(instruction_t *inst) {
	return inst->instrinput[0].val.uint8 != 14;
}


/**
 * Test if the instruction set the status register (argument 1).
 * @param inst	Instruction to test.
 * @return		True if the SR is set, false else.
 */
static bool isSettingS(instruction_t *inst) {
	return inst->instrinput[1].val.uint8;
}


/**
 * Test if B instruction is linked.
 * @param inst	Instruction to test.
 * @return		True if it is a linked branch, false else.
 */
static bool isLinked(instruction_t *inst) {
	return inst->instrinput[1].val.uint8;
}


/**
 * Test if the given instruction (of ident ID_R_) is a load or a store.
 * @param inst	Instruction to test.
 * @return		True if it is load, false else.
 */
/*static bool isLoad(instruction_t *inst) {
	return inst->instrinput[5].val.uint8;
}*/


/**
 * Test if the given instruction (if ident ID_R_) is incremented or not.
 * @param inst	Instruction to test.
 * @return		True if it is load, false else.
 */
/*static bool isIncremented(instruction_t *inst) {
	return inst->instrinput[5].val.uint8
		|| inst->instrinput[5].val.uint8;
}*/

// Process class
class Process: public otawa::loader::old_gliss::Process {

public:
	Process(Manager *manager, hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 4; }

	void decodeRegs(
		otawa::Inst *oinst,
		elm::genstruct::AllocatedTable<hard::Register *>& in,
		elm::genstruct::AllocatedTable<hard::Register *>& out
	);

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

protected:
	virtual void decodeRegs(void) {
		((Process&)process()).decodeRegs(this, in_regs, out_regs);
	}
};


// BranchInst class
class BranchInst: public otawa::loader::old_gliss::BranchInst {
public:

	inline BranchInst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::old_gliss::BranchInst(process, kind, addr) { }
		
	virtual size_t size(void) const { return 4; }
	
protected:
	virtual address_t decodeTargetAddress(void);
	virtual void decodeRegs(void) {
		((Process&)process()).decodeRegs(this, in_regs, out_regs);
	}
};


// Plaform class
static hard::Platform::Identification PFID("arm-*-*");
class Platform: public hard::Platform {
public:
	Platform(const PropList& props): hard::Platform(PFID, props) {
		setBanks(otawa::arm::banks);
	}
};


/**
 */
void Process::decodeRegs(
	otawa::Inst *oinst,
	elm::genstruct::AllocatedTable<hard::Register *>& in,
	elm::genstruct::AllocatedTable<hard::Register *>& out
) {

	// Decode the instruction
	Address addr = oinst->address();
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch(addr.offset(), buffer);
	inst = iss_decode((state_t *)state(), addr.offset(), buffer);
	
	// Count registers
	int read_cnt = 0, written_cnt = 0;
	switch(inst->ident) {
	case ID_Instrunknown:
		break;
	cnt_uses:
		if(isSettingS(inst))
			written_cnt++;
	cnt_guarded:
		if(isConditional(inst))
			read_cnt++;
		break;

	case ID_TST__0: case ID_CMP__0: case ID_TEQ__0: case ID_CMN__0:
		read_cnt++;
	case ID_TST__1: case ID_CMP__1: case ID_TEQ__1: case ID_CMN__1:
		read_cnt++;
	case ID_TST_: case ID_CMP_: case ID_TEQ_: case ID_CMN_:
		read_cnt++;
		written_cnt++;
		goto cnt_guarded;

	case ID_MOV__0: case ID_MVN__0:
		read_cnt++;
	case ID_MOV__1: case ID_MVN__1:
		read_cnt++;
	case ID_MOV_: case ID_MVN_:
		 written_cnt++;
		 goto cnt_uses;

	case ID_SUB__0: case ID_SBC__0: case ID_RSC__0: case ID_RSB__0:
	case ID_ORR__0: case ID_EOR__0: case ID_BIC__0: case ID_AND__0:
	case ID_ADD__0: case ID_ADC__0:
		read_cnt++;
	case ID_SUB__1: case ID_SBC__1: case ID_RSC__1: case ID_RSB__1:
	case ID_ORR__1: case ID_EOR__1: case ID_BIC__1: case ID_AND__1:
	case ID_ADD__1: case ID_ADC__1:
		read_cnt++;
    case ID_SUB_: case ID_SBC_: case ID_RSC_: case ID_RSB_:
    case ID_ORR_: case ID_EOR_: case ID_BIC_: case ID_AND_:
    case ID_ADD_: case ID_ADC_:
    	read_cnt++;
    	written_cnt++;
    	goto cnt_uses;

	case ID_UMLAL_: case ID_SMLAL_:
	case ID_UMULL_: case ID_SMULL_:
		read_cnt += 2;
		written_cnt += 2;
		goto cnt_uses;

	 case ID_MLA_:
	 case ID_MUL_:
    	read_cnt += 2;
    	written_cnt += 1;
    	goto cnt_uses;

    case ID_CLZ_:
    	read_cnt++;
    	written_cnt++;
    	goto cnt_guarded;

    case ID_SWPB_: case ID_SWP_:
    	read_cnt += 2;
    	written_cnt++;
    	goto cnt_guarded;

    case ID_B_:
    	written_cnt++;
    	if(isLinked(inst))
    		written_cnt++;
    	goto cnt_guarded;

    case ID_BX_:
    	written_cnt += 2;
    	read_cnt++;
    	goto cnt_guarded;

    case ID_R_:
    	read_cnt++;
    	if(inst->instrinput[5].val.uint8)
    		read_cnt++;
    	else
    		written_cnt++;
    	read_cnt++;
    	if(inst->instrinput[4].val.uint8)
    		written_cnt++;
    	goto cnt_guarded;
    
    case ID_R__0:
    	if(inst->instrinput[5].val.uint8)
    		read_cnt++;
    	else
    		written_cnt++;
    	read_cnt++;
    	if(inst->instrinput[4].val.uint8)
    		written_cnt++;
    	goto cnt_guarded;

    case ID_LDRSH_: case ID_LDRSB_: case ID_LDRH_:
    	read_cnt++;
    	written_cnt++;
    	if(inst->instrinput[3].val.uint8)
        	written_cnt++;
    	else
    		read_cnt++;
    	goto cnt_guarded;
    
    case ID_STRH_:
    	read_cnt += 2;
    	if(inst->instrinput[3].val.uint8)
        	written_cnt++;
    	else
    		read_cnt++;
    	goto cnt_guarded;
    
	case ID_LDRSH__0: case ID_LDRSB__0: case ID_LDRH__0:
    	written_cnt++;
    	if(inst->instrinput[3].val.uint8)
        	written_cnt++;
    	else
    		read_cnt++;
    	goto cnt_guarded;
	
	case ID_STRH__0:
    	written_cnt++;
    	if(inst->instrinput[3].val.uint8)
        	written_cnt++;
    	else
    		read_cnt++;
    	goto cnt_guarded;

	case ID_MRS_SPSR: case ID_MRS_CPSR:
		read_cnt++;
		written_cnt++;
		goto cnt_guarded;

	case ID_MSR_CPSR_F_: case ID_MSR_CPSR_S_: case ID_MSR_CPSR_X_:
	case ID_MSR_CPSR_C_: case ID_MSR_CPSR_FS_: case ID_MSR_CPSR_FX_:
	case ID_MSR_CPSR_SX_: case ID_MSR_CPSR_SC_:
	case ID_MSR_CPSR_XC_: case ID_MSR_CPSR_FSX_: case ID_MSR_CPSR_FXC_:
	case ID_MSR_CPSR_SXC_: case ID_MSR_CPSR_FSXC_: case ID_MSR_CPSR_:
	case ID_MSR_SPSR_F_: case ID_MSR_SPSR_S_: case ID_MSR_SPSR_X_:
	case ID_MSR_SPSR_C_: case ID_MSR_SPSR_FS_: case ID_MSR_SPSR_FX_:
    case ID_MSR_SPSR_SX_: case ID_MSR_SPSR_SC_: case ID_MSR_SPSR_XC_:
    case ID_MSR_SPSR_FSX_: case ID_MSR_SPSR_FXC_: case ID_MSR_SPSR_SXC_:
    case ID_MSR_SPSR_FSXC_: case ID_MSR_SPSR_:
	case ID_MSR_SPSR_F__0: case ID_MSR_CPSR_F__0: 
    case ID_MSR_CPSR_S__0: case ID_MSR_CPSR_X__0: case ID_MSR_CPSR_C__0:
    case ID_MSR_CPSR_FS__0: case ID_MSR_CPSR_FX__0:
    case ID_MSR_CPSR_SX__0: case ID_MSR_CPSR_SC__0: case ID_MSR_CPSR_XC__0:
    case ID_MSR_CPSR_FSX__0: case ID_MSR_CPSR_FXC__0: case ID_MSR_CPSR_SXC__0:
    case ID_MSR_CPSR_FSXC__0: case ID_MSR_CPSR__0: 
    case ID_MSR_SPSR_S__0: case ID_MSR_SPSR_X__0: case ID_MSR_SPSR_C__0:
    case ID_MSR_SPSR_FS__0: case ID_MSR_SPSR_FX__0:
    case ID_MSR_SPSR_SX__0: case ID_MSR_SPSR_SC__0: case ID_MSR_SPSR_XC__0:
    case ID_MSR_SPSR_FSX__0: case ID_MSR_SPSR_FXC__0: case ID_MSR_SPSR_SXC__0:
    case ID_MSR_SPSR_FSXC__0: case ID_MSR_SPSR__0:
	case ID_MSR_CPSR_F__1: case ID_MSR_SPSR_F__1:
	case ID_MSR_CPSR_F__2: case ID_MSR_SPSR_F__2:
    	written_cnt++;
    	read_cnt++;
    	goto cnt_guarded;
    	
    case ID_SWI_:
    	goto cnt_guarded;
		 
    case ID_M_:
    	if(inst->instrinput[4].val.uint8) {
    		written_cnt += countQuad(inst->instrinput[10].val.uint8);
    		written_cnt += countQuad(inst->instrinput[9].val.uint8);
    		written_cnt += countQuad(inst->instrinput[8].val.uint8);
    		written_cnt += countQuad(inst->instrinput[7].val.uint8);
    	}
    	else {
    		read_cnt += countQuad(inst->instrinput[10].val.uint8);
    		read_cnt += countQuad(inst->instrinput[9].val.uint8);
    		read_cnt += countQuad(inst->instrinput[8].val.uint8);
    		read_cnt += countQuad(inst->instrinput[7].val.uint8);    		
    	}
    	read_cnt++;
    	if(inst->instrinput[3].val.uint8)
    		written_cnt++;
    	if(inst->instrinput[2].val.uint8)
    		written_cnt++;
    	goto cnt_guarded;
	}
	
	// Record registers
	int i = 0, j = 0;
	in.allocate(read_cnt);
	out.allocate(written_cnt);
	switch(inst->ident) {
	case ID_Instrunknown:
		break;
	rec_uses:
		if(isSettingS(inst))
			out[j++] = &sr;
	rec_guarded:
		if(isConditional(inst))
			in[i++] = &sr;
		break;

	case ID_TST__0: case ID_CMP__0: case ID_TEQ__0: case ID_CMN__0:
		in[i++] = gpr[inst->instrinput[2].val.uint8];		
	case ID_TST__1: case ID_CMP__1: case ID_TEQ__1: case ID_CMN__1:
		in[i++] = gpr[inst->instrinput[4].val.uint8];
	case ID_TST_: case ID_CMP_: case ID_TEQ_: case ID_CMN_:
		out[j++] = &sr;
		in[i++] = gpr[inst->instrinput[1].val.uint8];
		goto rec_guarded;

	case ID_MOV__0: case ID_MVN__0:
		in[i++] = gpr[inst->instrinput[2].val.uint8];		
	case ID_MOV__1: case ID_MVN__1:
		in[i++] = gpr[inst->instrinput[5].val.uint8];		
	case ID_MOV_: case ID_MVN_:
		out[j++] = gpr[inst->instrinput[2].val.uint8];
		goto rec_uses;

	case ID_SUB__0: case ID_SBC__0: case ID_RSC__0: case ID_RSB__0:
	case ID_ORR__0: case ID_EOR__0: case ID_BIC__0: case ID_AND__0:
	case ID_ADD__0: case ID_ADC__0:
		in[i++] = gpr[inst->instrinput[4].val.uint8];		
	case ID_SUB__1: case ID_SBC__1: case ID_RSC__1: case ID_RSB__1:
	case ID_ORR__1: case ID_EOR__1: case ID_BIC__1: case ID_AND__1:
	case ID_ADD__1: case ID_ADC__1:
		in[i++] = gpr[inst->instrinput[6].val.uint8];		
    case ID_SUB_: case ID_SBC_: case ID_RSC_: case ID_RSB_:
    case ID_ORR_: case ID_EOR_: case ID_BIC_: case ID_AND_:
    case ID_ADD_: case ID_ADC_:
		in[i++] = gpr[inst->instrinput[2].val.uint8];		
		out[j++] = gpr[inst->instrinput[3].val.uint8];
    	goto rec_uses;

	case ID_UMLAL_: case ID_SMLAL_:
	case ID_UMULL_: case ID_SMULL_:
		out[j++] = gpr[inst->instrinput[2].val.uint8];
		out[j++] = gpr[inst->instrinput[3].val.uint8];
		in[i++] = gpr[inst->instrinput[4].val.uint8];		
		in[i++] = gpr[inst->instrinput[5].val.uint8];		
		goto rec_uses;
	
    case ID_MUL_: case ID_MLA_:
		out[j++] = gpr[inst->instrinput[2].val.uint8];
		in[i++] = gpr[inst->instrinput[3].val.uint8];		
		in[i++] = gpr[inst->instrinput[4].val.uint8];		
    	goto rec_uses;
		
    case ID_CLZ_:
		out[j++] = gpr[inst->instrinput[1].val.uint8];
		in[i++] = gpr[inst->instrinput[2].val.uint8];		
    	goto rec_guarded;

    case ID_SWPB_: case ID_SWP_:
		out[j++] = gpr[inst->instrinput[2].val.uint8];
		in[i++] = gpr[inst->instrinput[3].val.uint8];		
		in[i++] = gpr[inst->instrinput[1].val.uint8];		
    	goto rec_guarded;

    case ID_B_:
    	out[j++] = gpr[pc];
    	if(isLinked(inst))
    		out[j++] = gpr[lr];
    	goto rec_guarded;

    case ID_BX_:
    	out[j++] = gpr[pc];
    	out[j++] = &sr;
    	in[i++] = gpr[inst->instrinput[1].val.uint8];
    	goto rec_guarded;

    case ID_R_:
    	in[i++] = gpr[inst->instrinput[10].val.uint8];
    	if(inst->instrinput[5].val.uint8)
    		in[i++] = gpr[inst->instrinput[7].val.uint8];
    	else
    		out[j++] = gpr[inst->instrinput[7].val.uint8];
		in[i++] = gpr[inst->instrinput[6].val.uint8];
    	if(inst->instrinput[4].val.uint8)
    		out[j++] = gpr[inst->instrinput[6].val.uint8];
    	goto rec_guarded;
    
    case ID_R__0:
    	if(inst->instrinput[5].val.uint8)
    		in[i++] = gpr[inst->instrinput[7].val.uint8];
    	else
    		out[j++] = gpr[inst->instrinput[7].val.uint8];
		in[i++] = gpr[inst->instrinput[6].val.uint8];
    	if(inst->instrinput[4].val.uint8)
    		out[j++] = gpr[inst->instrinput[6].val.uint8];
    	goto rec_guarded;
    	
    case ID_LDRSH_: case ID_LDRSB_: case ID_LDRH_:
    	in[i++] = gpr[inst->instrinput[6].val.uint8];
    	out[j++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[3].val.uint8)
        	out[j++] = gpr[inst->instrinput[4].val.uint8];
    	else
    		in[i++] = gpr[inst->instrinput[4].val.uint8];
    	goto rec_guarded;
    
    case ID_STRH_:
    	in[i++] = gpr[inst->instrinput[6].val.uint8];
    	in[i++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[3].val.uint8)
        	out[j++] = gpr[inst->instrinput[4].val.uint8];
    	else
    		in[i++] = gpr[inst->instrinput[4].val.uint8];
    	goto rec_guarded;
    
	case ID_LDRSH__0: case ID_LDRSB__0: case ID_LDRH__0:
    	out[j++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[3].val.uint8)
        	out[j++] = gpr[inst->instrinput[4].val.uint8];
    	else
    		in[i++] = gpr[inst->instrinput[4].val.uint8];
    	goto rec_guarded;
	
	case ID_STRH__0:
    	out[j++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[3].val.uint8)
        	out[j++] = gpr[inst->instrinput[4].val.uint8];
    	else
    		in[i++] = gpr[inst->instrinput[4].val.uint8];
    	goto rec_guarded;

	case ID_MRS_SPSR: case ID_MRS_CPSR:
    	out[j++] = gpr[inst->instrinput[1].val.uint8];
		in[i++] = &sr;
		goto cnt_guarded;

	case ID_MSR_CPSR_F_: case ID_MSR_CPSR_S_: case ID_MSR_CPSR_X_:
	case ID_MSR_CPSR_C_: case ID_MSR_CPSR_FS_: case ID_MSR_CPSR_FX_:
	case ID_MSR_CPSR_SX_: case ID_MSR_CPSR_SC_:
	case ID_MSR_CPSR_XC_: case ID_MSR_CPSR_FSX_: case ID_MSR_CPSR_FXC_:
	case ID_MSR_CPSR_SXC_: case ID_MSR_CPSR_FSXC_: case ID_MSR_CPSR_:
	case ID_MSR_SPSR_F_: case ID_MSR_SPSR_S_: case ID_MSR_SPSR_X_:
	case ID_MSR_SPSR_C_: case ID_MSR_SPSR_FS_: case ID_MSR_SPSR_FX_:
    case ID_MSR_SPSR_SX_: case ID_MSR_SPSR_SC_: case ID_MSR_SPSR_XC_:
    case ID_MSR_SPSR_FSX_: case ID_MSR_SPSR_FXC_: case ID_MSR_SPSR_SXC_:
    case ID_MSR_SPSR_FSXC_: case ID_MSR_SPSR_:
	case ID_MSR_SPSR_F__0: case ID_MSR_CPSR_F__0: 
    case ID_MSR_CPSR_S__0: case ID_MSR_CPSR_X__0: case ID_MSR_CPSR_C__0:
    case ID_MSR_CPSR_FS__0: case ID_MSR_CPSR_FX__0:
    case ID_MSR_CPSR_SX__0: case ID_MSR_CPSR_SC__0: case ID_MSR_CPSR_XC__0:
    case ID_MSR_CPSR_FSX__0: case ID_MSR_CPSR_FXC__0: case ID_MSR_CPSR_SXC__0:
    case ID_MSR_CPSR_FSXC__0: case ID_MSR_CPSR__0: 
    case ID_MSR_SPSR_S__0: case ID_MSR_SPSR_X__0: case ID_MSR_SPSR_C__0:
    case ID_MSR_SPSR_FS__0: case ID_MSR_SPSR_FX__0:
    case ID_MSR_SPSR_SX__0: case ID_MSR_SPSR_SC__0: case ID_MSR_SPSR_XC__0:
    case ID_MSR_SPSR_FSX__0: case ID_MSR_SPSR_FXC__0: case ID_MSR_SPSR_SXC__0:
    case ID_MSR_SPSR_FSXC__0: case ID_MSR_SPSR__0:
	case ID_MSR_CPSR_F__1: case ID_MSR_SPSR_F__1:
	case ID_MSR_CPSR_F__2: case ID_MSR_SPSR_F__2:
    	in[i++] = gpr[inst->instrinput[1].val.uint8];
		out[i++] = &sr;
		goto rec_guarded;

	case ID_SWI_:
    	goto rec_guarded;
    	
    case ID_M_:
    	if(inst->instrinput[4].val.uint8) {
    		recordQuad(inst->instrinput[10].val.uint8, 0, out, j);
    		recordQuad(inst->instrinput[9].val.uint8, 4, out, j);
    		recordQuad(inst->instrinput[8].val.uint8, 8, out, j);
    		recordQuad(inst->instrinput[7].val.uint8, 12, out, j);
    	}
    	else {
    		recordQuad(inst->instrinput[10].val.uint8, 0, in, i);
    		recordQuad(inst->instrinput[9].val.uint8, 4, in, i);
    		recordQuad(inst->instrinput[8].val.uint8, 8, in, i);
    		recordQuad(inst->instrinput[7].val.uint8, 12, in, i);
    	}
    	in[i++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[3].val.uint8)
        	out[j++] = gpr[inst->instrinput[5].val.uint8];
    	if(inst->instrinput[2].val.uint8)
    		out[j++] = &sr;
    	goto rec_guarded;
	}
}


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
	bool is_mla = false;
	bool is_multiple = false;
	otawa::Inst *res = 0;
	switch(inst->ident) {

	case ID_Instrunknown:
		//cerr << addr << ": unknown\n";
		res = new Inst(*this, 0, addr);
		break;

	 case ID_MOV__1:
		 if(inst->instrinput[2].val.uint8 == pc
		 && inst->instrinput[5].val.uint8 == lr
		 && inst->instrinput[3].val.uint8 == 0) {
			 kind |= Inst::IS_RETURN;
			 goto branch;
		 }
		
	case ID_UMLAL_: case ID_SMLAL_:
	case ID_UMULL_: case ID_SMULL_:
	 case ID_MLA_:
		 is_mla = true;
    case ID_MUL_:
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
    	is_multiple = true;
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
    		res = new Inst(*this, kind, addr);
        	break;
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
		res = new BranchInst(*this, kind, addr);
		break;
	}
	
	// Set the annotations
	if(is_mla)
		IS_MLA(res) = true;
	if(is_multiple)
		IS_MULTIPLE_LOAD_STORE(res) = true;
	return res;
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
	return new Process(man, new Platform(props), props);
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
