/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	PowerPC plugin implementation
 */

#include <elm/assert.h>
#include <otawa/loader/new_gliss/Process.h>
#include <otawa/loader/new_gliss/BranchInst.h>
#include <otawa/prog/Loader.h>
#include <otawa/platform.h>
#include <otawa/hard/Register.h>
#include <gel/gel.h>
#define ISS_DISASM
#include "emul.h"

using namespace otawa::hard;

extern "C" gel_file_t *loader_file(memory_t* memory);

#define TRACE(m) //cout << m << io::endl
#define RTRACE(m)	//m
//#define SCAN_ARGS

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

protected:
	virtual otawa::Inst *decode(address_t addr);
	virtual void *gelFile(void) {
		return loader_file(((state_t *)state())->M);
	}
};

// Inst class
class Inst: public otawa::loader::new_gliss::Inst {
public:

	inline Inst(Process& process, kind_t kind, address_t addr)
		: otawa::loader::new_gliss::Inst(process, kind, addr) { }
		
	virtual size_t size(void) const { return 4; }

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
		
	virtual size_t size(void) const { return 4; }
	
protected:
	virtual address_t decodeTargetAddress(void);
	virtual void decodeRegs(void) {
		((Process&)process()).decodeRegs(this, &in_regs, &out_regs);
	}
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
const PlainBank Platform::GPR_bank("GPR", Register::INT,  32, "r%d", 32);


/**
 * FPR register bank.
 */
const PlainBank Platform::FPR_bank("FPR", Register::FLOAT, 64, "fr%d", 32);


/**
 * CR register bank
 */
const PlainBank Platform::CR_bank("CR", Register::BITS, 4, "cr%d", 8);


/**
 * CTR register
 */
hard::Register Platform::CTR_reg("ctr", Register::BITS, 32);


/**
 * LR register
 */
hard::Register Platform::LR_reg("lr", Register::ADDR, 32);


/**
 * XER register
 */
hard::Register Platform::XER_reg("xer", Register::INT, 32);


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
}


/**
 */
otawa::Inst *Process::decode(address_t addr) {

	// Decode the instruction
	code_t buffer[20];
	instruction_t *inst;
	//cerr << "ADDR " << addr << io::endl;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode((state_t *)state(), (::address_t)addr, buffer);

	// Build the instruction
	otawa::Inst *result;	
	if(inst->ident == ID_Instrunknown) {
		result = new Inst(*this, 0, addr);
		//cerr << "UNKNOWN !!!\n" << result << io::endl;
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
			//cerr << "A BRANCH !!!\n";
			/*if(inst->ident == ID_BL_ && inst->instrinput[0].val.Int24 == 1) {
				result = new Inst(*this, kind, addr);
				break;
			}*/
			switch(inst->ident) {
			case ID_BL_:
			case ID_BLA_:
				kind |= Inst::IS_CALL;
				break;
			case ID_BCL_:
			case ID_BCLA_:
			case ID_BCCTRL_:
				kind |= Inst::IS_CALL | Inst::IS_COND;
				break;
			case ID_BC_:
			case ID_BCA_:
			case ID_BCCTR_:
				kind |= Inst::IS_COND;
				break;
			case ID_BCLR_:
				if(inst->instrinput[0].val.Uint5 == 20
				&& inst->instrinput[1].val.Uint5 == 0) {
					kind |= Inst::IS_RETURN;
					break;
				}
			case ID_BCLRL_:
				kind |= (Inst::IS_RETURN | Inst::IS_COND);
				break;
			}
			result = new BranchInst(*this, kind, addr);
			//cerr << "BRANCH " << result << " : " << io::hex(result->kind()) << io::endl;
			break;
		default:
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
	iss_fetch((::address_t)address(), buffer);
	inst = iss_decode((state_t *)process().state(), (::address_t)address(), buffer);

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
		_kind |= IS_COND;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = address() + (inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_BCLA_:
	case ID_BCA_:
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = (address_t)(inst->instrinput[2].val.Int14 << 2);
		break;
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
	iss_fetch((::address_t)(unsigned long)oinst->address(), buffer);
	inst = iss_decode((state_t *)state(), (::address_t)oinst->address(), buffer /*, 0*/);
	if(inst->ident == ID_Instrunknown) {
		/* in_regs = new elm::genstruct::AllocatedTable<hard::Register *>(0);
		out_regs = new elm::genstruct::AllocatedTable<hard::Register *>(0); */
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
static CString table[] = {
	"elf_20"
};
static elm::genstruct::Table<CString> ppc_aliases(table, 1);
 

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

} }	// otawa::ppc


// PowerPC GLISS Loader entry point
otawa::ppc::Loader OTAWA_LOADER_HOOK;
otawa::ppc::Loader& ppc_plugin = OTAWA_LOADER_HOOK;
