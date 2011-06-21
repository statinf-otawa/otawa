/*
 *	$Id$
 *	PowerPC OTAWA plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#include <otawa/prog/Manager.h>
#include <otawa/loader/new_gliss/Process.h>
#include <otawa/loader/new_gliss/BranchInst.h>
#include <otawa/prog/Loader.h>
#include <otawa/platform.h>
#include <otawa/hard/Register.h>
#include <gel/gel.h>
#include <otawa/util/FlowFactLoader.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/sim/features.h>
#include <otawa/prog/sem.h>
#define ISS_DISASM
#include <emul.h>
#include <iss_include.h>
#include "InstPattern.h"

using namespace otawa::hard;

extern "C" gel_file_t *loader_file(memory_t* memory);

#define TRACE(m) //cerr << m << io::endl
#define RTRACE(m)	//m
//#define SCAN_ARGS

// Trace for switch parsing
#define STRACE(m)	//cerr << m << io::endl

// External memory accesses
extern uint32_t Mem_Base_Read_First;
extern uint32_t Mem_Base_Write_First;
extern uint32_t Mem_Base_Read_Last;
extern uint32_t Mem_Base_Write_Last;

// Kind Table
static unsigned long kinds[] = {
	otawa::Inst::IS_ALU | otawa::Inst::IS_INT,					// ARITH = "0"
	otawa::Inst::IS_ALU | otawa::Inst::IS_INT,					// MULDIV = "1"
	otawa::Inst::IS_ALU | otawa::Inst::IS_INT,					// INTCMP = "2"
	otawa::Inst::IS_ALU | otawa::Inst::IS_INT,					// LOGIC = "3"
	otawa::Inst::IS_ALU | otawa::Inst::IS_SHIFT | otawa::Inst::IS_INT,	// SHIFTROT = "4"
	otawa::Inst::IS_MEM | otawa::Inst::IS_STORE | otawa::Inst::IS_INT,	// STORE = "5"
	otawa::Inst::IS_MEM | otawa::Inst:: IS_LOAD | otawa::Inst::IS_INT,	// LOAD = "6"
	otawa::Inst::IS_INTERN,								// MEMSYNC = "7"
	otawa::Inst::IS_CONTROL,								// BRANCH = "8"
	otawa::Inst::IS_INTERN,								// CRLI = "9"
	otawa::Inst::IS_CONTROL | otawa::Inst::IS_TRAP,									// SYSTEM = "10"
	otawa::Inst::IS_CONTROL | otawa::Inst::IS_TRAP,									// TRAP = "11"
	otawa::Inst::IS_INTERN,								// EXT = "12"
	otawa::Inst::IS_INTERN,								// CONTROL = "13"
	otawa::Inst::IS_INTERN,								// CACHE = "14"
	otawa::Inst::IS_INTERN,								// SEG = "15"
	otawa::Inst::IS_INTERN,								// TLB = "16"
	otawa::Inst::IS_ALU | otawa::Inst::IS_FLOAT,					// FPARITH = "17"
	otawa::Inst::IS_ALU | otawa::Inst::IS_MUL | otawa::Inst::IS_FLOAT,	// FPMUL = "18"
	otawa::Inst::IS_ALU | otawa::Inst::IS_DIV | otawa::Inst::IS_FLOAT,	// FPDIV = "19"
	otawa::Inst::IS_ALU | otawa::Inst::IS_FLOAT,					// FPMADD = "20"
	otawa::Inst::IS_ALU | otawa::Inst::IS_FLOAT | otawa::Inst::IS_INT,	// FPRC = "21"
	otawa::Inst::IS_MEM | otawa::Inst::IS_LOAD | otawa::Inst::IS_FLOAT,	// FPLOAD = "22"
	otawa::Inst::IS_MEM | otawa::Inst::IS_STORE | otawa::Inst::IS_FLOAT,	// FPSTORE = "23"
	otawa::Inst::IS_INTERN | otawa::Inst::IS_FLOAT,				// FPSCRI = "24"
	otawa::Inst::IS_ALU | otawa::Inst::IS_FLOAT,					// FPCMP = "25"
	otawa::Inst::IS_ALU | otawa::Inst::IS_FLOAT					// FPMOV = "26"
};


namespace otawa { namespace ppc {

// Platform class
class Platform: public hard::Platform {
public:
	static const Identification ID;
	Platform(const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Registers
	static hard::Register CTR_reg;
	static hard::Register LR_reg;
	static hard::Register XER_reg;
	static const hard::PlainBank GPR_bank;
	static const hard::PlainBank FPR_bank;
	static const hard::PlainBank CR_bank;
	static const hard::MeltedBank MISC_bank;

	// otawa::Platform overload
	virtual bool accept(const Identification& id);
};

// SimState class
class SimState: public otawa::SimState {
public:
  SimState(Process *process, state_t *state, bool _free = false)
    : otawa::SimState(process) {
		ASSERT(process);
		ASSERT(state);
		  _state = new state_t;
		  *_state = *state;
	}
	virtual ~SimState(void) {
	    delete _state;
	}
  virtual void setSP(const Address& addr) {
    state()->gpr[1] = addr.offset();
  }
	inline state_t *state(void) const { return _state; }
	virtual Inst *execute(Inst *oinst) {
		ASSERTP(oinst, "null instruction pointer");

		Address addr = oinst->address();
		code_t buffer[20];
		instruction_t *inst;
		NIA(_state) = addr.address();
		iss_fetch(addr.address(), buffer);
		inst = iss_decode(_state, addr.address(), buffer);
		iss_complete(inst, _state);
		iss_free(inst);
		if(NIA(_state) == *(oinst->topAddress())) {
			Inst *next = oinst->nextInst();
			while(next && next->isPseudo())
				next = next->nextInst();
			if(next && next->address() == Address(NIA(_state)))
				return next;
		}
		Inst *next = process()->findInstAt(NIA(_state));
		ASSERTP(next, "cannot find instruction at " << (void *)NIA(_state) << " from " << oinst->address());
		return next;
	}

	// memory accesses
	virtual Address lowerRead(void) { return Mem_Base_Read_First; }
	virtual Address upperRead(void) { return Mem_Base_Read_Last; }
	virtual Address lowerWrite(void) { return Mem_Base_Write_First; }
	virtual Address upperWrite(void) { return Mem_Base_Write_Last; }

private:
	state_t *_state;
};


// Process class
class Process: public otawa::loader::new_gliss::Process {

public:
	Process(Manager *manager, hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 4; }
	void decodeRegs(
		Inst *inst,
		elm::genstruct::AllocatedTable<hard::Register *> *in,
		elm::genstruct::AllocatedTable<hard::Register *> *out);
	virtual otawa::SimState *newState(void) {
	  return new SimState(this, (state_t *)state(), true);
	}

	virtual Address initialSP(void) const {
		return ((state_t *)state())->gpr[1];
	}
protected:
	virtual otawa::Inst *decode(address_t addr);
	virtual void *gelFile(void) {
		return loader_file(((state_t *)state())->M);
	}
	virtual void *memory(void) { return ((state_t *)state())->M; }
};

// Inst class
class Inst: public otawa::loader::new_gliss::Inst {
public:

	inline Inst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::new_gliss::Inst(process, kind, addr) { }

	virtual size_t size(void) const { return 4; }
	virtual void semInsts(sem::Block& block);

protected:
	virtual void decodeRegs(void) {
		((Process&)process()).decodeRegs(this, &in_regs, &out_regs);
	}
};


// BranchInst class
class BranchInst: public otawa::loader::new_gliss::BranchInst {
public:

	inline BranchInst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::new_gliss::BranchInst(process, kind, addr) { }

	virtual void semInsts(sem::Block& block);
	virtual size_t size(void) const { return 4; }

protected:
	virtual address_t decodeTargetAddress(void);
	virtual void decodeRegs(void) {
		((Process&)process()).decodeRegs(this, &in_regs, &out_regs);
	}
	Option<Address> checkFarCall(address_t call);
	bool checkSwitch(void);

private:
	void do_branch(sem::Block& block, unsigned long off, bool abs, bool link);
	void do_branch_cond(sem::Block& block, unsigned long off, int bo, int bi, bool abs, bool link);
};


/**
 * Register banks.
 */
static const RegBank *banks[] = {
	&Platform::GPR_bank,
	&Platform::FPR_bank,
	&Platform::CR_bank,
	&Platform::MISC_bank
};
static const elm::genstruct::Table<const RegBank *> banks_table(banks, 4);


/**
 * GPR register bank.
 */
const PlainBank Platform::GPR_bank("GPR", hard::Register::INT,  32, "r%d", 32);


/**
 * FPR register bank.
 */
const PlainBank Platform::FPR_bank("FPR", hard::Register::FLOAT, 64, "fr%d", 32);


/**
 * CR register bank
 */
const PlainBank Platform::CR_bank("CR", hard::Register::BITS, 4, "cr%d", 8);


/**
 * CTR register
 */
hard::Register Platform::CTR_reg("ctr", hard::Register::BITS, 32);


/**
 * LR register
 */
hard::Register Platform::LR_reg("lr", hard::Register::ADDR, 32);


/**
 * XER register
 */
hard::Register Platform::XER_reg("xer", hard::Register::INT, 32);


/**
 * MISC register bank
 */
const hard::MeltedBank Platform::MISC_bank("MISC", &Platform::CTR_reg,
	&Platform::LR_reg, &Platform::XER_reg, 0);


/**
 * Identification of the default platform.
 */
const Platform::Identification Platform::ID("powerpc-elf-");


/**
 * Build a new gliss platform with the given configuration.
 * @param props		Configuration properties.
 */
Platform::Platform(const PropList& props): hard::Platform(ID, props) {
	setBanks(banks_table);
}


/**
 * Build a new platform by cloning.
 * @param platform	Platform to clone.
 * @param props		Configuration properties.
 */
Platform::Platform(const Platform& platform, const PropList& props)
: hard::Platform(platform, props) {
	setBanks(banks_table);
}


/**
 */
bool Platform::accept(const Identification& id) {
	return id.abi() == "elf" && id.architecture() == "powerpc";
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
): otawa::loader::new_gliss::Process(manager, pf, props) {
	provide(CONTROL_DECODING_FEATURE);
	provide(REGISTER_USAGE_FEATURE);
	provide(MEMORY_ACCESSES);
	provide(SEMANTICS_INFO);
}


/**
 */
otawa::Inst *Process::decode(address_t addr) {

	// Decode the instruction
	code_t buffer[20];
	instruction_t *inst;
	TRACE("ADDR " << addr);
	iss_fetch(addr.address(), buffer);
	inst = iss_decode((state_t *)state(), addr.address(), buffer);

	// Build the instruction
	otawa::Inst *result = 0;
	if(inst->ident == ID_Instrunknown) {
		result = new Inst(*this, 0, addr);
		TRACE("UNKNOWN !!!\n" << result);
		iss_free(inst);
		return result;
	}
	else {

		// Compute the kind
		assert(iss_table[inst->ident].category <= 26);
		Inst::kind_t kind = kinds[iss_table[inst->ident].category];

		// Build the instruction
		switch(iss_table[inst->ident].category) {
		case 1:		// MULDIV
			switch(inst->ident) {
			case ID_DIVWU_R_R_R: case ID_DIVWU_D_R_R_R: case ID_DIVWUO_R_R_R:
			case ID_DIVWUO_D_R_R_R: case ID_DIVW_R_R_R: case ID_DIVW_D_R_R_R:
			case ID_DIVWO_R_R_R: case ID_DIVWO_D_R_R_R:
				kind |= Inst::IS_DIV;
				break;
			case ID_MULHWU_R_R_R: case ID_MULHWU_D_R_R_R: case ID_MULHW_R_R_R:
			case ID_MULHW_D_R_R_R: case ID_MULLI_R_R_: case ID_MULLW_R_R_R:
			case ID_MULLW_D_R_R_R: case ID_MULLWO_R_R_R: case ID_MULLWO_D_R_R_R:
				kind |= Inst::IS_MUL;
				break;
			}
			result = new Inst(*this, kind, addr);
			break;
		case 8:		// BRANCH
		case 10:	// SYSTEM
		case 11:	// TRAP
			switch(inst->ident) {
			case ID_BL_:
				ASSERT(inst->instrinput[0].type == PARAM_INT24_T);
				if(inst->instrinput[0].val.Int24 == 1) {
					kind = Inst::IS_ALU | Inst::IS_INT;
					goto simple;
				}
			case ID_BLA_:
				kind |= Inst::IS_CALL;
				break;
			case ID_BCL_:
				//ASSERT(inst->instrinput[2].type == PARAM_INT16_T);
				if(inst->instrinput[2].val.Int16 == 1) {
					kind = Inst::IS_ALU | Inst::IS_INT;
					goto simple;
				}
			case ID_BCLA_:
				kind |= Inst::IS_CALL | Inst::IS_COND;
				break;
			case ID_BCCTRL_:
				kind |= Inst::IS_CALL;
				if(!(inst->instrinput[0].val.Uint5 == 20
				&& inst->instrinput[1].val.Uint5 == 0))
					kind |= Inst::IS_COND;
				break;
			case ID_BC_:
			case ID_BCA_:
			case ID_BCCTR_:
				if(!(inst->instrinput[0].val.Uint5 == 20
				&& inst->instrinput[1].val.Uint5 == 0))
					kind |= Inst::IS_COND;
				break;
			case ID_BCLR_:
				kind |= Inst::IS_RETURN;
				if((inst->instrinput[0].val.Uint5 & 0x14) != 0x14) // 0b1x1xx
					kind |= Inst::IS_COND;
				break;
			case ID_BCLRL_:
				kind |= Inst::IS_CALL;
				if((inst->instrinput[0].val.Uint5 & 0x14) != 0x14) // 0b1x1xx
					kind |= Inst::IS_COND;
				break;
			}
			if(!result) {
				kind |= Inst::IS_CONTROL;
				result = new BranchInst(*this, kind, addr);
			}
			break;
		default:
		simple:
			result = new Inst(*this, kind, addr);
			break;
		}
	}

	// Cleanup
	ASSERT(result);
	iss_free(inst);
	return result;
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

	// Explore the instruction
	address_t target_addr = 0;
	switch(inst->ident) {
	case ID_BL_:
	case ID_B_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = address() + (inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BLA_:
	case ID_BA_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = (address_t)(inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BCL_:
	case ID_BC_:
		//_kind |= IS_COND;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = address() + (inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_BCLA_:
	case ID_BCA_:
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = (address_t)(inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_BCCTR_:
		target_addr = 0;
		checkSwitch();
		break;
	case ID_BCCTRL_: {
			iss_free(inst);
			Option<Address> addr = checkFarCall(address());
			if(addr.isOne()) {
				target_addr = *addr;
			}
		}
		return target_addr;
	}

	// Return result
	iss_free(inst);
	/*cerr << "TARGET OF [" << address() << "] "
		 << this << " : " << target_addr << io::endl;*/
	return target_addr;
}


// Platform definition
static hard::Platform::Identification PFID("ppc-*-*");


// Register scanning table
typedef enum conv_t {
	NO_SUPPORT = 0,
	TO_BANK,
	TO_REG,
	TO_CR
} conv_t;

typedef struct param_t {
	conv_t type;
	union {
		hard::Register *reg;
		const hard::RegBank *bank;
	} data;
	#ifdef SCAN_ARGS
		CString name;
	#endif
} param_t;

class ScanArgs {
	param_t ids[VOID_T + 1];
public:

	ScanArgs(void) {

		// Default initialization
		for(int i = 0; i < VOID_T + 1; i++) {
			ids[i].type = NO_SUPPORT;
			#ifdef SCAN_ARGS
				ids[i].name = "???";
			#endif
		}

		// Supported register initialization
		ids[GPR_T].type = TO_BANK;
		ids[GPR_T].data.bank = &Platform::GPR_bank;
		ids[FPR_T].type = TO_BANK;
		ids[FPR_T].data.bank = &Platform::FPR_bank;
		ids[CR_T].type = TO_CR;
		ids[CR_T].data.bank = &Platform::CR_bank;
		ids[CTR_T].type = TO_REG;
		ids[CTR_T].data.reg = &Platform::CTR_reg;
		ids[XER_T].type = TO_REG;
		ids[XER_T].data.reg = &Platform::XER_reg;
		ids[LR_T].type = TO_REG;
		ids[LR_T].data.reg = &Platform::LR_reg;

		// Name initialization
		#ifdef SCAN_ARGS
	    	ids[PARAM_UINT3_T].name = "PARAM_UINT3_T";
			ids[PARAM_UINT4_T].name = "PARAM_UINT4_T";
			ids[PIA_T].name = "PIA_T";
			ids[MSR_T].name = "MSR_T";
			ids[CIA_T].name = "CIA_T";
			ids[NIA_T].name = "NIA_T";
			ids[FPSCR_T].name = "FPSCR_T";
			ids[PARAM_UINT5_T].name = "PARAM_UINT5_T";
			ids[CR_T].name = "CR_T";
			ids[FPR_T].name = "FPR_T";
			ids[GPR_T].name = "GPR_T";
			ids[M_T].name = "M_T";
			ids[PARAM_INT16_T].name = "PARAM_INT16_T";
			ids[PARAM_INT24_T].name = "PARAM_INT24_T";
			ids[LR_T].name = "LR_T";
			ids[CTR_T].name = "CTR_T";
			ids[PARAM_INT14_T].name = "PARAM_INT14_T";
			ids[XER_T].name = "XER_T";
			ids[PARAM_UINT1_T].name = "PARAM_UINT1_T";
			ids[PARAM_UINT16_T].name = "PARAM_UINT16_T";
			ids[PARAM_UINT8_T].name = "PARAM_UINT8_T";
			ids[PARAM_UINT9_T].name = "PARAM_UINT9_T";
			ids[PARAM_UINT10_T].name = "PARAM_UINT10_T";
			ids[TBL_T].name = "TBL_T";
			ids[TB_T].name = "TB_T";
			ids[TBU_T].name = "TBU_T";
			ids[DSISR_T].name = "DSISR_T";
			ids[DAR_T].name = "DAR_T";
			ids[DEC_T].name = "DEC_T";
			ids[SDR1_T].name = "SDR1_T";
			ids[SRR_T].name = "SRR_T";
			ids[SPRG_T].name = "SPRG_T";
			ids[EAR_T].name = "EAR_T";
			ids[PVR_T].name = "PVR_T";
			ids[IBAT_T].name = "IBAT_T";
			ids[DBAT_T].name = "DBAT_T";
			ids[UMMCR_T].name = "UMMCR_T";
			ids[UPMC_T].name = "UPMC_T";
			ids[USIA_T].name = "USIA_T";
			ids[MMCR_T].name = "MMCR_T";
			ids[PMC_T].name = "PMC_T";
			ids[SIA_T].name = "SIA_T";
			ids[DMISS_T].name = "DMISS_T";
			ids[DCMP_T].name = "DCMP_T";
			ids[HASH_T].name = "HASH_T";
			ids[IMISS_T].name = "IMISS_T";
			ids[ICMP_T].name = "ICMP_T";
			ids[RPA_T].name = "RPA_T";
			ids[HID_T].name = "HID_T";
			ids[IABR_T].name = "IABR_T";
			ids[DABR_T].name = "DABR_T";
			ids[L2PM_T].name = "L2PM_T";
			ids[L2CR_T].name = "L2CR_T";
			ids[ICTC_T].name = "ICTC_T";
			ids[THRM_T].name = "THRM_T";
			ids[SR_T].name = "SR_T";
			ids[VOID_T].name = "VOID_T";
		#endif
	}

	#ifdef SCAN_ARGS
		void decode(io::Output& out, ii_t *param) {
			out << ids[param->type].name << '(' << param->val.int32 << ')';
		}

		void decodeParams(io::Output& out, ii_t *params) {
			bool first = true;
			for(int i = 0; ; i++) {
				out << "\t[" << i << "] = ";
				decode(out, params + i);
				out << '\n';
				if(params[i].type == VOID_T)
					break;
			}
		}
	#endif

	bool isSupported(ii_t *param) {
		switch(ids[param->type].type) {
		case NO_SUPPORT:
			return false;
		case TO_CR:
			return param->val.uint8 != 255;
		default:
			return true;
		}
	}

	hard::Register *reg(ii_t *param) {
		switch(ids[param->type].type) {
		case NO_SUPPORT:
			return 0;
		case TO_REG:
			return ids[param->type].data.reg;
		case TO_CR:
			if(param->val.uint8 == 255)
				return 0;
		case TO_BANK:
			return ids[param->type].data.bank->get(param->val.uint8);
		default:
			assert(0);
			return 0;
		}
	}
};
static ScanArgs scan_args;


/**
 */
void Process::decodeRegs(
	otawa::Inst *oinst,
	elm::genstruct::AllocatedTable<hard::Register *> *in,
	elm::genstruct::AllocatedTable<hard::Register *> *out
) {

	// Decode instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch(oinst->address().address(), buffer);
	inst = iss_decode((state_t *)state(), oinst->address().address(), buffer);
	if(inst->ident == ID_Instrunknown) {
		/* in_regs = new elm::genstruct::AllocatedTable<hard::Register *>(0);
		out_regs = new elm::genstruct::AllocatedTable<hard::Register *>(0); */
		iss_free(inst);
		return;
	}

	// Select r0 linked to immediate 0
	/* !!TODO!! This implementation of zero-cabled register R0 is quite crude
	 * and surely non-portable (be careful to future changes in GLISS)
	 * It should be fixed in a way or in another. Difficult di publish the
	 * PPC loader in this conditions...
	 */
	bool no_reg = iss_table[inst->ident].user0;

	// Count read registers
	RTRACE(dump(cout); cout << io::endl);
	int cnt = 0, i, j;
	for(i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instrinput + i)
		&& (!no_reg || cnt != 0 || inst->instrinput[i].val.uint8)) {
			RTRACE(cout << "count " << i << io::endl);
			cnt++;
		}
	switch(inst->ident) {
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
		RTRACE(cout << "count " << i << io::endl);
		cnt++;
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:
	case ID_MCRF_CRF_CRF:
		RTRACE(cout << "count " << i << io::endl);
		cnt++;
	}
	in->allocate(cnt);
	elm::genstruct::AllocatedTable<hard::Register *> *tab = in;

	// Get read registers
	#ifdef SCAN_ARGS
		cout << "INSTRINPUT\n" << cnt << ": ";
		scan_args.decodeParams(cout, inst->instrinput);
		cout << '\n';
	#endif
	for(i = 0, j = 0; inst->instrinput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instrinput + i);
		if(reg && (!no_reg || cnt != 0 || inst->instrinput[i].val.uint8)) {
			RTRACE(cout << "set " << i << io::endl);
			tab->set(j++, reg);
		}
	}
	switch(inst->ident) {
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:
		RTRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		break;
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
		RTRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[2].val.uint8) / 4]);
		assert(tab->get(j - 1));
	case ID_MCRF_CRF_CRF:
		RTRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[1].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;
	}

	// Count the write registers
	cnt = 0;
	for(i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instroutput + i)) {
			RTRACE(cout << "count " << i << io::endl);
			cnt++;
		}
	switch(inst->ident) {
	case ID_CMPI_R_: case ID_CMP_R_R: case ID_CMPLI_R_: case ID_CMPL_R_R:
	case ID_FCMPU_CRF_FR_FR: case ID_FCMPO_CRF_FR_FR:
	case ID_MCRXR_CRF: case ID_MCRFS_CRF_CRF:
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
	case ID_MCRF_CRF_CRF:
		RTRACE(cout << "count " << i << io::endl);
		cnt++;
		break;
	}
	out->allocate(cnt);
	tab = out;

	// Get write registers
	#ifdef SCAN_ARGS
		cout << "INSTROUTPUT\n" << cnt << ": ";
		scan_args.decodeParams(cout, inst->instroutput);
		cout << '\n';
	#endif
	for(i = 0, j = 0; inst->instroutput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instroutput + i);
		if(reg) {
			RTRACE(cout << "set " << i << io::endl);
			tab->set(j++, reg);
		}
	}
	switch(inst->ident) {
	case ID_CMPI_R_: case ID_CMP_R_R: case ID_CMPLI_R_: case ID_CMPL_R_R:
	case ID_FCMPU_CRF_FR_FR: case ID_FCMPO_CRF_FR_FR:
	case ID_MCRXR_CRF: case ID_MCRFS_CRF_CRF:
		RTRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
	case ID_MCRF_CRF_CRF:
		RTRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;
	}
	assert(j == cnt);

	// Free instruction
	iss_free(inst);
}

// otawa::loader::ppc::Loader class
class Loader: public otawa::Loader {
public:
	Loader(void);

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *load(Manager *_man, CString path, const PropList& props);
	virtual otawa::Process *create(Manager *_man, const PropList& props);
};


// Alias table
static string table[] = {
	"elf_20"
};
static elm::genstruct::Table<string> ppc_aliases(table, 1);


/**
 * Build a new loader.
 */
Loader::Loader(void)
: otawa::Loader("ppc", Version(1, 0, 0), OTAWA_LOADER_VERSION, ppc_aliases) {
}


/**
 * Get the name of the loader.
 * @return Loader name.
 */
CString Loader::getName(void) const {
	return "ppc";
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


/**
 * Check for far-call pattern.
 * @param call	Address of the far call.
 * @return		If the pattern matches, the called address.
 */
Option<Address> BranchInst::checkFarCall(address_t call) {

	// call:	bcctrl 20,0
	// call-4:	mtspr 288,ri
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch(call.address() - 4, buffer);
	inst = iss_decode((state_t *)process().state(), call.address() - 4, buffer);
	if(inst->ident != ID_MTSPR_R
	|| inst->instrinput[1].val.uint16 != 288) {
		iss_free(inst);
		return none;
	}
	int i = inst->instrinput[0].val.uint8;
	iss_free(inst);

	// call-8:	addi ri,ri,low
	iss_fetch(call.address() - 8, buffer);
	inst = iss_decode((state_t *)process().state(), call.address() - 8, buffer);
	if(inst->ident != ID_ADDI_R_R_
	|| inst->instrinput[0].val.uint8 != i
	|| inst->instrinput[1].val.uint8 != i) {
		iss_free(inst);
		return none;
	}
	unsigned long low = inst->instrinput[2].val.int16;
	iss_free(inst);

	// call-12:	addis ri,r0,up
	iss_fetch(call.address() - 12, buffer);
	inst = iss_decode((state_t *)process().state(), call.address() - 12, buffer);
	if(inst->ident != ID_ADDIS_R_R_
	|| inst->instrinput[0].val.uint8 != i
	|| inst->instrinput[1].val.uint8 != 0) {
		iss_free(inst);
		return none;
	}
	signed long up = inst->instrinput[2].val.int16;
	iss_free(inst);

	// Called address: (up << 16) + low
	return Address((up << 16) + low);
}


using namespace otawa::loader;

template <int n> class R: public Reg<n> { };
template <int n> class C: public ConstUInt8<n> { };
template <int n> class C16: public ConstUInt16<n> { };

bool BranchInst::checkSwitch(void) {
	Address addr = topAddress();
	typedef UInt16<0> size;
	typedef UInt16<1> table_hi;
	typedef Int16<2> table_lo;
	typedef UInt16<2> base_hi;
	typedef Int16<3> base_lo;
	typedef ConstReg<0> zero;

	// Short switch template
	typedef Seq<
		Seq <
			I<ID_ADDI_R_R_, R<0>, R<1>, Ignore >,		// R1 = switch value, R0 = fixed switch value
			I<ID_CMPLI_R_, C<0>, C<0>, R<0>, size >,
			I<ID_BC_, C<12>, C<1>, Ignore >
		>,
		Seq <
			I<ID_ADDIS_R_R_, R<2>, zero, table_hi >,
			I<ID_ADDI_R_R_, R<2>, R<2>, table_lo >,				// R2 = table address
			I<ID_RLWINM_R_R_, R<3>, R<0>, C<2>, C<0>, C<29> >,	// R3 = table offset
			I<ID_LWZX_R_R_R, R<4>, R<2>, R<3> >					// R4 = branch offset
		>,
		Seq <
			I<ID_ADDIS_R_R_, R<5>, zero, base_hi >,
			I<ID_ADDI_R_R_, R<5>, R<5>, base_lo >,				// R5 = branch base
			I<ID_ADD_R_R_R, R<6>, R<4>, R<5> >
		>,
		Seq <
			I<ID_MTSPR_R, R<6>, C16<288> >,
			I<ID_BCCTR_, C<20>, C<0> >
		>
	> short_switch;

	// Long switch template
	// ri = R<0>, rj = R<1>, rk = R<2>, rl = R<3>,
	// rm = R<4>, rn = R<5>, rp = R<6>, rq = R<7>,
	// ro = R<8>, rr = R<9>
	typedef Seq<
		Seq<
			I<ID_ADDI_R_R_, R<0>, Ignore, Ignore>,
			I<ID_CMPLI_R_, C<0>, C<0>, R<0>, size >,
			I<ID_BC_, C<12>, C<1>, Ignore >
		>,
		Seq<
			I<ID_OR_R_R_R, R<0>, R<1>, R<0> >,
			I<ID_RLWINM_R_R_, R<1>, R<2>, C<2>, C<0>, C<29> >,
			I<ID_ADDIS_R_R_, R<3>, zero, table_hi>,
			I<ID_ADDI_R_R_, R<4>, R<3>, table_lo>
		>,
		Seq<
			I<ID_ADD_R_R_R, R<5>, R<2>, R<4> >,
			I<ID_LWZ_R_R_, R<6>, R<5>, C<0> >,
			I<ID_ADDIS_R_R_, R<7>, zero, base_hi>,
			I<ID_ADDI_R_R_, R<8>, R<7>, base_lo >
		>,
		Seq<
			I<ID_ADD_R_R_R, R<9>, R<6>, R<8> >,
			I<ID_MTSPR_R, R<9>, C16<288> >,
			I<ID_BCCTR_, C<20>, C<0> >
		>
	> long_switch;

	// Indirect switch template
	typedef UInt16<2> jump_hi, def_hi;
	typedef Int16<3> jump_lo, def_lo;
	typedef Seq<
		Seq<													// before:
			I<ID_ADDI_R_R_, R<0>, R<1> >,						//	addi	x0, x1, *
			I<ID_RLWINM_R_R_, R<0>, R<2>, C<0>, C<24>, C<31> >,	//	rlwinm	x2, x0, 0, 24, 31
			I<ID_CMPLI_R_, C<0>, C<0>, R<2>, size>,				//	cmpli	0,0, x2, table_max
			I<ID_BC_, C<12>, C<1>, C<10> >						//	bc		12, 1, 10
		>,
		Seq<													// cases:
			I<ID_ADDIS_R_R_, R<3>, zero, jump_hi >,				//	addis	x3, r0, jump_high
			I<ID_ADDI_R_R_, R<3>, R<3>, jump_lo >,				//	addi	x3, x3, jump_low
			I<ID_ADDIS_R_R_, R<4>, zero, table_hi >,			//	addis	x4, r0, table_high
			I<ID_ADDI_R_R_, R<4>, R<4>, table_lo >				//	addi	x4, x4, table_low
		>,
		Seq<
			I<ID_RLWINM_R_R_, R<0>, R<5>, C<2>, C<0>, C<29> >,	//	rlwinm	x5, x0, 2, 0, 29
			I<ID_LWZX_R_R_R, R<6>, R<4>, R<5> >,				//	lwzx	x6, x4, x5
			I<ID_RLWINM_R_R_, R<6>, R<6>, C<2>, C<0>, C<29> >,	//	rlwinm	x6, x6, 2, 0, 29
			I<ID_LWZX_R_R_R, R<7>, R<3>, R<6> >					//	lwzx	x7, x3, x6
		>,
		I<ID_B_, C<3> >,										//	b		3
		Seq<													// default:
			I<ID_ADDIS_R_R_, R<8>, zero, def_hi>,				//	addis	x8, r0, def_high
			I<ID_ADDI_R_R_, R<7>, R<8>, def_lo>,				//	addi	x7, x8, def_low
																// branch:
			I<ID_MTSPR_R, R<7>, C16<288> >,						//	mtspr	288, x7
			I<ID_BCCTR_, C<20>, C<0> >							//	bcctr	20, 0
		>
	> indirect_switch;

	// Check template
	addr = topAddress();
	STRACE("look for short switch at " << addr);
	short_switch::reset();
	bool res = short_switch::matchBack((state_t *)process().state(), addr);
	if(!res) {
		addr = topAddress();
		STRACE("look for long switch at " << addr);
		long_switch::reset();
		res = long_switch::matchBack((state_t *)process().state(), addr);
	}

	// Record the information
	if(res) {
		genstruct::SortedSLList<Address> addresses;
		STRACE("switch at " << addr);
		Address table = (table_hi::value() << 16) + table_lo::value();
		Address base = (base_hi::value() << 16) + base_lo::value();
		STRACE("table = " << table << ", base = " << base);
		for(int i = 0; i <= size::value(); i++) {
			signed long offset;
			process().get(table + i * 4, offset);
			Address addr = base.offset() + offset;
			STRACE("\tbranch to " << addr << ", offset=" << (void *)offset);
			if(!addresses.contains(addr)) {
				addresses.add(addr);
				BRANCH_TARGET(this).add(addr);
			}
		}
		STRACE(io::endl);
		return true;
	}

	// Check indirect switch template
	indirect_switch::reset();
	addr = topAddress();
	STRACE("look for indirect switch at " << addr);
	res = indirect_switch::matchBack((state_t *)process().state(), addr);
	if(res) {
		genstruct::SortedSLList<Address> addresses;
		STRACE("indirect switch at " << addr);

		// Add the default branch
		Address target = (def_hi::value() << 16) + def_lo::value();
		addresses.add(target);
		BRANCH_TARGET(this).add(target);

		// Process tables
		Address table = (table_hi::value() << 16) + table_lo::value();
		Address jump = (jump_hi::value() << 16) + jump_lo::value();
		STRACE("table = " << table << ", jump = " << jump);
		for(int i = 0; i <= size::value(); i++) {

			// compute the target
			unsigned long index;
			process().get(table + i * 4, index);
			unsigned long offset;
			process().get(jump + t::uint32(index) * 4, offset);
			Address target = offset;

			// add the target
			if(!addresses.contains(target)) {
				STRACE("\tbranch to " << target << ", index=" << index << io::endl);
				addresses.add(target);
				BRANCH_TARGET(this).add(target);
			}
		}
		return true;
	}

	// Template not recognized
	STRACE("no switch at " << addr << io::endl);
	return false;
}

// fast generation
#define arg(i)	inst->instrinput[i].val.uint8
#define r(i)	Platform::GPR_bank[arg(i)]->platformNumber()
#define fr(i)	Platform::FPR_bank[arg(i)]->platformNumber()
#define	cr(i)	Platform::CR_bank[i]->platformNumber()
#define i(i)	inst->instrinput[i].val.int16
#define ib(i)	inst->instrinput[i].val.int8
#define il(i)	inst->instrinput[i].val.int32
#define t1		(-1)
#define t2		(-2)
#define	lr		Platform::LR_reg.platformNumber()
#define	ctr		Platform::CTR_reg.platformNumber()

static void sem_load_indexed(sem::Block& block, instruction_t *inst, int size, bool update, bool flt) {
	if(!update && !arg(1))
		block.add(sem::load(flt ? fr(0) : r(0), r(2), size));
	else {
		block.add(sem::add(t1, r(1), r(2)));
		block.add(sem::load(flt ? fr(0) : r(0), t1, size));
		if(update)
			block.add(sem::set(r(1), t1));
	}
}

static void sem_store_indexed(sem::Block& block, instruction_t *inst, int size, bool update, bool flt) {
	if(!update && !arg(1))
		block.add(sem::store(flt ? fr(0) : r(0), r(2), size));
	else {
		block.add(sem::add(t1, r(1), r(2)));
		block.add(sem::store(flt ? fr(0) : r(0), t1, size));
		if(update)
			block.add(sem::set(r(1), t1));
	}
}

static void sem_load_immediate(sem::Block& block, instruction_t *inst, int size, bool update, bool flt) {
	block.add(sem::seti(t2, i(2)));
	if(!update && !arg(1))
		block.add(sem::load(r(0), t2, size));
	else {
		block.add(sem::add(t1, r(1), t2));
		block.add(sem::load(flt ? fr(0) : r(0), t1, size));
		if(update)
			block.add(sem::set(r(1), t1));
	}
}

static void sem_store_immediate(sem::Block& block, instruction_t *inst, int size, bool update, bool flt) {
	block.add(sem::seti(t2, i(2)));
	if(!update && !arg(1))
		block.add(sem::store(r(0), t2, size));
	else {
		block.add(sem::add(t1, r(1), t2));
		block.add(sem::store(flt ? fr(0) : r(0), t1, size));
		if(update)
			block.add(sem::set(r(1), t1));
	}
}


void Inst::semInsts(sem::Block& block)  {

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(address().address(), buffer);
	inst = iss_decode((state_t *)process().state(), address().address(), buffer);

	// scan the instruction
	switch(inst->ident) {

	// unknown effect on the stack
	case ID_Instrunknown:
		break;

	// no effect on the stack
	// system
	case ID_TLBIA: case ID_TLBIE_R: case ID_TLBSYNC:
	case ID_SYNC: case ID_ISYNC:
	case ID_EIEIO: case ID_ICBI_R_R:
	case ID_DCBA_R_R: case ID_DCBZ_R_R: case ID_DCBTST_R_R:
	case ID_DCBT_R_R: case ID_DCBST_R_R: case ID_DCBF_R_R: case ID_DCBI_R_R:
	// float
	case ID_MTFSFI_D_CRF_: case ID_MTFSFI_CRF_: case ID_MTFSF_D_FR: case ID_MTFSF_FR:
	case ID_MTFSB_CRB: case ID_MTFSB_CRB_0: case ID_MTFSB_D_CRB: case ID_MTFSB_D_CRB_0:
	case ID_MFFS_D_FR: case ID_MFFS_FR: case ID_MCRFS_CRF_CRF: case ID_FCTIWZ_D_FR_FR:
	case ID_FCTIWZ_FR_FR: case ID_FCTIW_D_FR_FR: case ID_FCTIW_FR_FR: case ID_FRSP_D_FR_FR:
	case ID_FRSP_FR_FR: case ID_FNABS_FR_FR: case ID_FNABS_D_FR_FR: case ID_FABS_FR_FR:
	case ID_FABS_D_FR_FR: case ID_FNEG_FR_FR: case ID_FNEG_D_FR_FR: case ID_FMR_FR_FR:
	case ID_FMR_D_FR_FR: case ID_FCMPU_CRF_FR_FR: case ID_FCMPO_CRF_FR_FR: case ID_FMSUB_FR_FR_FR_FR:
	case ID_FMSUB_D_FR_FR_FR_FR: case ID_FMSUBS_FR_FR_FR_FR: case ID_FMSUBS_D_FR_FR_FR_FR: case ID_FNMSUB_FR_FR_FR_FR:
	case ID_FNMSUB_D_FR_FR_FR_FR: case ID_FNMSUBS_FR_FR_FR_FR: case ID_FNMSUBS_D_FR_FR_FR_FR: case ID_FMADD_FR_FR_FR_FR:
	case ID_FMADD_D_FR_FR_FR_FR: case ID_FMADDS_FR_FR_FR_FR: case ID_FMADDS_D_FR_FR_FR_FR: case ID_FNMADD_FR_FR_FR_FR:
	case ID_FNMADD_D_FR_FR_FR_FR: case ID_FNMADDS_FR_FR_FR_FR: case ID_FNMADDS_D_FR_FR_FR_FR: case ID_FADD_FR_FR_FR:
	case ID_FADD_D_FR_FR_FR: case ID_FADDS_FR_FR_FR: case ID_FADDS_D_FR_FR_FR: case ID_FSUB_FR_FR_FR:
	case ID_FSUB_D_FR_FR_FR: case ID_FSUBS_FR_FR_FR: case ID_FSUBS_D_FR_FR_FR: case ID_FMUL_FR_FR_FR:
	case ID_FMUL_D_FR_FR_FR: case ID_FMULS_FR_FR_FR: case ID_FMULS_D_FR_FR_FR: case ID_FDIV_FR_FR_FR:
	case ID_FDIV_D_FR_FR_FR: case ID_FDIVS_FR_FR_FR: case ID_FDIVS_D_FR_FR_FR:
	// control
	case ID_BL_: case ID_BLA_: case ID_BA_: case ID_B_:
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:
	case ID_SC: case ID_RFI: case ID_TW_R_R: case ID_TWI_R_:
	// too complex
	case ID_LSWX_R_R_R: case ID_LSWI_R_R_:
	case ID_STSWX_R_R_R: case ID_STSWI_R_R_:
	case ID_ECOWX_R_R_R: case ID_ECIWX_R_R_R:
	case ID_STWCX_D_R_R_R:
	// condition codes
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB: case ID_CRXOR_CRB_CRB_CRB:
	case ID_CRNAND_CRB_CRB_CRB: case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
	case ID_MCRF_CRF_CRF: case ID_MCRXR_CRF: case ID_MTCRF_R:

		// special registers
	case ID_MTMSR_R: case ID_MTSR_R: case ID_MTSRIN_R_R:
		break;

	case ID_MTSPR_R: case ID_MTSPR_R_0:
		if(inst->instrinput[1].val.uint16 == 288)
			block.add(sem::set(ctr, r(0)));
		
    break;

	// comparisons
	case ID_CMP_R_R:
		block.add(sem::cmp(cr(ib(0)), r(2), r(3)));
		break;
	case ID_CMPI_R_:
		block.add(sem::seti(t1, i(3)));
		block.add(sem::cmp(cr(ib(0)), r(2), t1));
		break;
	case ID_CMPL_R_R:
		block.add(sem::cmpu(cr(ib(0)), r(2), r(3)));
		break;
	case ID_CMPLI_R_:
		block.add(sem::seti(t1, i(3)));
		block.add(sem::cmpu(cr(ib(0)), r(2), t1));
		break;

	// load indexed
	case ID_LFDUX_FR_R_R: sem_load_indexed(block, inst, 8, true, true); break;
	case ID_LFDX_FR_R_R:  sem_load_indexed(block, inst, 8, false, true); break;
	case ID_LFSUX_FR_R_R: sem_load_indexed(block, inst, 4, true, true); break;
	case ID_LFSX_FR_R_R: sem_load_indexed(block, inst, 4, false, true); break;
	case ID_LBZUX_R_R_R: sem_load_indexed(block, inst, 1, true, false); break;
	case ID_LBZX_R_R_R: sem_load_indexed(block, inst, 1, false, false); break;
	case ID_LHAUX_R_R_R: sem_load_indexed(block, inst, 2, true, false); break;
	case ID_LHAX_R_R_R: sem_load_indexed(block, inst, 2, false, false); break;
	case ID_LHZUX_R_R_R: sem_load_indexed(block, inst, 2, true, false); break;
	case ID_LHZX_R_R_R: sem_load_indexed(block, inst, 2, false, false); break;
	case ID_LWZUX_R_R_R: sem_load_indexed(block, inst, 4, true, false); break;
	case ID_LWZX_R_R_R: sem_load_indexed(block, inst, 4, false, false); break;
	case ID_LWBRX_R_R_R: sem_load_indexed(block, inst, 4, false, false); break;
	case ID_LHBRX_R_R_R: sem_load_indexed(block, inst, 2, false, false); break;
	case ID_LWARX_R_R_R: sem_load_indexed(block, inst, 4, false, false); break;

	// load immediate
	case ID_LFDU_FR_R_: sem_load_immediate(block, inst, 8, true, true); break;
	case ID_LFD_FR_R_: sem_load_immediate(block, inst, 8, false, true); break;
	case ID_LFSU_FR_R_: sem_load_immediate(block, inst, 4, true, true); break;
	case ID_LFS_FR_R_: sem_load_immediate(block, inst, 4, false, true); break;
	case ID_LBZU_R_R_: sem_load_immediate(block, inst, 1, true, false); break;
	case ID_LBZ_R_R_: sem_load_immediate(block, inst, 1, false, false); break;
	case ID_LHZU_R_R_: sem_load_immediate(block, inst, 2, true, false); break;
	case ID_LHAU_R_R_: sem_load_immediate(block, inst, 2, false, false); break;
	case ID_LHA_R_R_: sem_load_immediate(block, inst, 2, true, false); break;
	case ID_LHZ_R_R_: sem_load_immediate(block, inst, 2, false, false); break;
	case ID_LWZU_R_R_: sem_load_immediate(block, inst, 4, true, false); break;
	case ID_LWZ_R_R_: sem_load_immediate(block, inst, 4, false, false); break;

	// load multiple
	case ID_LMW_R_R_:
		block.add(sem::seti(t2, 4));
		block.add(sem::seti(t1, i(2)));
		if(r(1) != 0)
			block.add(sem::add(t1, t1, r(1)));
		for(int i = arg(0); i < 32; i++) {
			block.add(sem::load(Platform::GPR_bank[i]->platformNumber(), t1, 4));
			block.add(sem::add(t1, t1, t2));
		}
		break;

	// store indexed
	case ID_STFDUX_FR_R_R: sem_store_indexed(block, inst, 8, true, true); break;
	case ID_STFDX_FR_R_R: sem_store_indexed(block, inst, 8, false, true); break;
	case ID_STFSUX_FR_R_R: sem_store_indexed(block, inst, 4, true, true); break;
	case ID_STFSX_FR_R_R: sem_store_indexed(block, inst, 4, false, true); break;
	case ID_STBUX_R_R_R: sem_store_indexed(block, inst, 1, true, false); break;
	case ID_STBX_R_R_R: sem_store_indexed(block, inst, 1, false, false); break;
	case ID_STHUX_R_R_R: sem_store_indexed(block, inst, 2, true, false); break;
	case ID_STHX_R_R_R: sem_store_indexed(block, inst, 2, false, false); break;
	case ID_STWUX_R_R_R: sem_store_indexed(block, inst, 4, true, false); break;
	case ID_STWX_R_R_R: sem_store_indexed(block, inst, 4, false, false); break;
	case ID_STWBRX_R_R_R: sem_store_indexed(block, inst, 4, false, false); break;
	case ID_STHBRX_R_R_R: sem_store_indexed(block, inst, 2, false, false); break;

	// store immediate
	case ID_STFDU_FR_R_: sem_store_immediate(block, inst, 8, true, true); break;
	case ID_STFD_FR_R_: sem_store_immediate(block, inst, 8, false, true); break;
	case ID_STFSU_FR_R_: sem_store_immediate(block, inst, 4, true, true); break;
	case ID_STFS_FR_R_: sem_store_immediate(block, inst, 4, false, true); break;
	case ID_STBU_R_R_: sem_store_immediate(block, inst, 1, true, false); break;
	case ID_STB_R_R_: sem_store_immediate(block, inst, 1, false, false); break;
	case ID_STHU_R_R_: sem_store_immediate(block, inst, 2, true, false); break;
	case ID_STH_R_R_: sem_store_immediate(block, inst, 2, false, false); break;
	case ID_STWU_R_R_: sem_store_immediate(block, inst, 4, true, false); break;
	case ID_STW_R_R_: sem_store_immediate(block, inst, 4, false, false); break;

	// store multiple
	case ID_STMW_R_R_:
		block.add(sem::seti(t2, 4));
		block.add(sem::seti(t1, i(2)));
		if(r(1) != 0)
			block.add(sem::add(t1, t1, r(1)));
		for(int i = arg(0); i < 32; i++) {
			block.add(sem::store(Platform::GPR_bank[i]->platformNumber(), t1, 4));
			block.add(sem::add(t1, t1, t2));
		}
		break;

	// unsupported unary arithmetics
	case ID_NEGO_D_R_R: case ID_NEGO_R_R: case ID_NEG_D_R_R: case ID_NEG_R_R:
	case ID_MFCR_R: case ID_MFSPR_R_: case ID_MFTB_R_: case ID_MFSR_R_: case ID_MFMSR_R:
	case ID_MFSPR_R__0: case ID_MFSRIN_R_R:
	// unsupported binary arithmetics
	case ID_DIVWU_R_R_R: case ID_DIVWU_D_R_R_R: case ID_DIVWUO_R_R_R: case ID_DIVWUO_D_R_R_R:
	case ID_DIVW_R_R_R: case ID_DIVW_D_R_R_R: case ID_DIVWO_R_R_R: case ID_DIVWO_D_R_R_R:
	case ID_MULHWU_R_R_R: case ID_MULHWU_D_R_R_R: case ID_MULHW_R_R_R: case ID_MULHW_D_R_R_R:
	case ID_MULLI_R_R_: case ID_MULLW_R_R_R: case ID_MULLW_D_R_R_R: case ID_MULLWO_R_R_R:
	case ID_MULLWO_D_R_R_R:
	case ID_SUBF_D_R_R_R: case ID_SUBFO_R_R_R: case ID_SUBFO_D_R_R_R:
	case ID_SUBFC_R_R_R: case ID_SUBFC_D_R_R_R: case ID_SUBFCO_R_R_R: case ID_SUBFCO_D_R_R_R:
	case ID_SUBFE_R_R_R: case ID_SUBFE_D_R_R_R: case ID_SUBFEO_R_R_R: case ID_SUBFEO_D_R_R_R:
	case ID_SUBFIC_R_R_:
	case ID_SUBFME_R_R: case ID_SUBFME_D_R_R: case ID_SUBFMEO_R_R: case ID_SUBFMEO_D_R_R:
	case ID_SUBFZE_R_R: case ID_SUBFZE_D_R_R: case ID_SUBFZEO_R_R: case ID_SUBFZEO_D_R_R:
	case ID_ADDC_R_R_R: case ID_ADDC_D_R_R_R: case ID_ADDCO_R_R_R: case ID_ADDCO_D_R_R_R:
	case ID_ADDE_R_R_R: case ID_ADDE_D_R_R_R: case ID_ADDEO_R_R_R: case ID_ADDEO_D_R_R_R:
	case ID_ADDZE_R_R: case ID_ADDZE_D_R_R: case ID_ADDZEO_R_R:	case ID_ADDZEO_D_R_R:
	case ID_ADDME_R_R: case ID_ADDME_D_R_R:	case ID_ADDMEO_R_R:	case ID_ADDMEO_D_R_R:
		block.add(sem::scratch(r(0)));
		break;

	case ID_ADD_R_R_R:
		block.add(sem::add(r(0), r(1), r(2)));
		break;
	case ID_SUBF_R_R_R:
		block.add(sem::sub(r(0), r(2), r(1)));
		break;

	// unsupported arithmetics : target second argument ! (Too bad ppc.nml)
	case ID_ORC_R_R_R: case ID_ORC_D_R_R_R: case ID_ORIS_R_R_: case ID_ORI_R_R_:
	case ID_OR_D_R_R_R:
	case ID_ANDC_R_R_R: case ID_ANDC_D_R_R_R: case ID_ANDIS_D_R_R_: case ID_ANDI_D_R_R_:
	case ID_AND_D_R_R_R:
	case ID_XORIS_R_R_: case ID_XORI_R_R_: case ID_XOR_R_R_R: case ID_XOR_D_R_R_R:
	case ID_NOR_D_R_R_R: case ID_NOR_R_R_R:
	case ID_NAND_D_R_R_R: case ID_NAND_R_R_R:
	case ID_EQV_D_R_R_R: case ID_EQV_R_R_R:
	case ID_EXTSH_R_R: case ID_EXTSH_D_R_R:
	case ID_EXTSB_R_R: case ID_EXTSB_D_R_R:
	case ID_CNTLZW_D_R_R: case ID_CNTLZW_R_R:
	case ID_RLWIMI_R_R_: case ID_RLWIMI_D_R_R_:
	case ID_RLWNM_R_R_R_: case ID_RLWNM_D_R_R_R_:
	case ID_SRAW_R_R_R: case ID_SRAW_D_R_R_R: case ID_SRAWI_R_R_: case ID_SRAWI_D_R_R_:
	case ID_SRW_R_R_R: case ID_SRW_D_R_R_R:
	case ID_SLW_R_R_R: case ID_SLW_D_R_R_R:
		block.add(sem::scratch(r(1)));
		break;

	// supported arithmetics
	case ID_RLWINM_R_R_: case ID_RLWINM_D_R_R_:
		if(arg(3) < arg(4) && (arg(4) - arg(3) + 1) + arg(2) == 32) {
			block.add(sem::seti(t1, arg(2)));
			block.add(sem::shl(r(1), r(0), t1));
    } else {
			block.add(sem::scratch(r(1)));
    }	
    break;

	case ID_OR_R_R_R: case ID_AND_R_R_R:
		if(arg(0) == arg(2))
			block.add(sem::set(r(1), r(0)));
		else
			block.add(sem::scratch(r(1)));
		break;
	case ID_ADDI_R_R_:
		if(arg(1) == 0)
			block.add(sem::seti(r(0), i(2)));
		else {
			block.add(sem::seti(t1, i(2)));
			block.add(sem::add(r(0), r(1), t1));
		}
		break;
	case ID_ADDIC_R_R_: case ID_ADDIC_D_R_R_:
		block.add(sem::seti(t1, i(2)));
		block.add(sem::add(r(0), r(1), t1));
		break;
	case ID_ADDIS_R_R_:
		if(arg(1) == 0)
			block.add(sem::seti(r(0), long(i(2)) << 16));
		else {
			block.add(sem::seti(t1, long(i(2)) << 16));
			block.add(sem::add(r(0), r(1), t1));
		}
		break;
	}

	// clean up
	iss_free(inst);
}

void BranchInst::do_branch(sem::Block& block, unsigned long off, bool abs, bool link) {
	Address addr;
	if(link) {
		addr = address() + 4;
		block.add(sem::seti(lr, addr.offset()));
	}
	if(!abs)
		addr = address() + (t::uint32(off) << 2);
	else
		addr = off << 2;
	block.add(sem::seti(t1, addr.offset()));
	block.add(sem::branch(t1));
}

#define BO(i)		(bo & (1 << (4 - i)))
void BranchInst::do_branch_cond(sem::Block& block, unsigned long off, int bo, int bi, bool abs, bool link) {

	// if ¬ BO[2] then CTR ← CTR – 1
	if(!BO(2)) {
		block.add(sem::seti(t1, 1));
		block.add(sem::sub(ctr, ctr, t1));
		// add compare
		// add condition
	}

	// do the link
	if(link) {
		Address addr = address() + 4;
		block.add(sem::seti(lr, addr.offset()));
	}

	// (CTR ≠ 0) ⊕ BO[3])
	if(BO(0)) {
		block.add(sem::seti(t1, 0));
		block.add(sem::cmp(t2, ctr, t1));
		block.add(sem::_if(!BO(3) ? sem::NE : sem::EQ, t2, 1));
		block.add(sem::cont());
	}

	// (CR[BI] ≡ BO[1])
	if(BO(2)) {
		sem::cond_t cond;
		// cmp : if a < b then c ← 0b100 else if a > b then c ← 0b010 else c ← 0b001
		if(BO(1))	// inverted condition !
			switch(bi & 0x3) {
			case 0: cond = sem::GE; break;
			case 1: cond = sem::LE; break;
			case 2: cond = sem::NE; break;
			case 3: cond = sem::ANY_COND; break;
			}
		else
			switch(bi & 0x3) {
			case 0: cond = sem::LT; break;
			case 1: cond = sem::GT; break;
			case 2: cond = sem::EQ; break;
			case 3: cond = sem::ANY_COND; break;
			}
		block.add(sem::_if(cond, cr(bi >> 2), 1));
		block.add(sem::cont());
	}

	// perform the jump
	do_branch(block, off, abs, false);
}


void BranchInst::semInsts(sem::Block& block)  {
	Address addr;

	// Decode the instruction
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch(address().address(), buffer);
	inst = iss_decode((state_t *)process().state(), address().address(), buffer);

	// scan the instruction
	switch(inst->ident) {
	case ID_B_:
		do_branch(block, il(0), false, false);
		break;
	case ID_BA_:
		do_branch(block, il(0), true, false);
		break;
	case ID_BL_:
		do_branch(block, il(0), false, true);
		break;
	case ID_BLA_:
		do_branch(block, il(0), true, true);
		break;
	case ID_BC_:
		do_branch_cond(block, i(2), ib(0), ib(1), false, false);
		break;
	case ID_BCA_:
		do_branch_cond(block, i(2), ib(0), ib(1), true, false);
		break;
	case ID_BCL_:
		do_branch_cond(block, i(2), ib(0), ib(1), false, true);
		break;
	case ID_BCLA_:
		do_branch_cond(block, i(2), ib(0), ib(1), true, true);
		break;

	case ID_BCLR_:
	case ID_BCLRL_:

	case ID_BCCTRL_:
	case ID_BCCTR_:
	case ID_SC:
		block.add(sem::trap(sem::ANY_COND));
		break;
	}

	// clean up
	iss_free(inst);
}

} }	// otawa::ppc


// PowerPC GLISS Loader entry point
otawa::ppc::Loader OTAWA_LOADER_HOOK;
otawa::ppc::Loader& ppc_plugin = OTAWA_LOADER_HOOK;
