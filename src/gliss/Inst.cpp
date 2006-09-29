/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	gliss/Instruction.cpp -- gliss::Instruction class implementation.
 */

#include <elm/io.h>
#include <otawa/gliss.h>

using namespace elm;

#ifndef NDEBUG
//#	define SCAN_ARGS
//#	define TRACE(c) c
#	define TRACE(c)
#else
#	define TRACE(c)
#endif

namespace otawa { namespace gliss {

// Kind Table
static unsigned long kinds[] = {
	Inst::IS_ALU | Inst::IS_INT,					// ARITH = "0"
	Inst::IS_ALU | Inst::IS_INT,					// MULDIV = "1"		!!TODO!!
	Inst::IS_ALU | Inst::IS_INT,					// INTCMP = "2"
	Inst::IS_ALU | Inst::IS_INT,					// LOGIC = "3"
	Inst::IS_ALU | Inst::IS_SHIFT | Inst::IS_INT,	// SHIFTROT = "4"
	Inst::IS_MEM | Inst::IS_STORE | Inst::IS_INT,	// STORE = "5"
	Inst::IS_MEM | Inst:: IS_LOAD | Inst::IS_INT,	// LOAD = "6"
	Inst::IS_INTERN,								// MEMSYNC = "7"
	Inst::IS_CONTROL,								// BRANCH = "8"
	Inst::IS_INTERN,								// CRLI = "9"
	Inst::IS_TRAP,									// SYSTEM = "10"
	Inst::IS_TRAP,									// TRAP = "11"
	Inst::IS_INTERN,								// EXT = "12"
	Inst::IS_INTERN,								// CONTROL = "13"
	Inst::IS_INTERN,								// CACHE = "14"
	Inst::IS_INTERN,								// SEG = "15"
	Inst::IS_INTERN,								// TLB = "16"
	Inst::IS_ALU | Inst::IS_FLOAT,					// FPARITH = "17"
	Inst::IS_ALU | Inst::IS_MUL | Inst::IS_FLOAT,	// FPMUL = "18"
	Inst::IS_ALU | Inst::IS_DIV | Inst::IS_DIV,		// FPDIV = "19"
	Inst::IS_ALU | Inst::IS_FLOAT,					// FPMADD = "20"
	Inst::IS_ALU | Inst::IS_FLOAT | Inst::IS_INT,	// FPRC = "21"
	Inst::IS_MEM | Inst::IS_LOAD | Inst::IS_FLOAT,	// FPLOAD = "22"
	Inst::IS_MEM | Inst::IS_STORE | Inst::IS_FLOAT,	// FPSTORE = "23"
	Inst::IS_INTERN | Inst::IS_FLOAT,				// FPSCRI = "24"
	Inst::IS_ALU | Inst::IS_FLOAT,					// FPCMP = "25"
	Inst::IS_ALU | Inst::IS_FLOAT					// FPMOV = "26"
};


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
 * @class Inst <otawa/gliss/Inst.h>
 * Representation of instructions in Gliss.
 */


/**
 * Build a new generic instruction.
 * @param segment	Owner segment.
 * @param address	Instruction address.
 */
Inst::Inst(CodeSegment& segment, address_t address)
: seg(segment), addr(address), flags(0), reads(0), writes(0) {
}


/**
 */
Inst::~Inst(void) {
	if(reads && reads != &no_regs)
		delete (elm::genstruct::AllocatedTable<hard::Register *> *)reads;
	if(writes && writes != &no_regs)
		delete (elm::genstruct::AllocatedTable<hard::Register *> *)writes;
}


// Overloaded
address_t Inst::address(void) {
	return addr;
}


// Overloaded
size_t Inst::size(void) {
	return 4;
}


// Overloaded
void Inst::dump(io::Output& out) {
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)addr, buffer);
	inst = iss_decode(seg.file().state(), (::address_t)addr, buffer);
	iss_disasm(out_buffer, inst);
	out << out_buffer;
	iss_free(inst);
}


/**
 * Scan the instruction for filling the object.
 */
void Inst::scan(void) {

	// Already computed?
	if(flags & BUILT)
		return;
	flags |= BUILT;
	
	// Get the instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)address(), buffer);
	inst = iss_decode(seg.file().state(), (::address_t)address(), buffer);
	assert(inst);
	
	// Intialize the category
	if(inst->ident == ID_Instrunknown)
		flags = 0;
	else {
		assert(iss_table[inst->ident].category <= 26);
		flags = kinds[iss_table[inst->ident].category];
	}
	
	// Call customization
	scanCustom(inst);
	
	// Cleanup
	iss_free(inst);
	/*cout << io::hex(address()) << " ";
	dump(cout);
	cout << "=>" << io::hex(flags) << io::endl;*/	
}


// Overload
const elm::genstruct::Table<hard::Register *>& Inst::readRegs(void) {
	if(!reads)
		scanRegs();
	return *reads;
}


// Overload
const elm::genstruct::Table<hard::Register *>& Inst::writtenRegs(void) {
	if(!writes)
		scanRegs();
	return *writes;
}


/**
 * Scan the register used by the statement.
 */
void Inst::scanRegs(void) {

	// Decode instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)addr, buffer);
	inst = iss_decode(seg.file().state(), (::address_t)addr, buffer);
	if(inst->ident == ID_Instrunknown) {
		reads = new elm::genstruct::AllocatedTable<hard::Register *>(0);
		writes = new elm::genstruct::AllocatedTable<hard::Register *>(0);
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
	TRACE(dump(cout); cout << io::endl);
	int cnt = 0, i, j;
	for(i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instrinput + i)
		&& (!no_reg || cnt != 0 || inst->instrinput[i].val.uint8)) {
			TRACE(cout << "count " << i << io::endl);
			cnt++;
		}
	switch(inst->ident) {
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
		TRACE(cout << "count " << i << io::endl);
		cnt++;			
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:	
	case ID_MCRF_CRF_CRF:
		TRACE(cout << "count " << i << io::endl);
		cnt++;		
	}
	elm::genstruct::AllocatedTable<hard::Register *> *tab =
		new elm::genstruct::AllocatedTable<hard::Register *>(cnt);

	// Get read registers
	#ifdef SCAN_ARGS
		cout << "INSTRINPUT\n" << cnt << ": ";
		scan_args.decodeParams(cout, inst->instrinput);
		cout << '\n';
	#endif
	for(i = 0, j = 0; inst->instrinput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instrinput + i);
		if(reg && (!no_reg || cnt != 0 || inst->instrinput[i].val.uint8)) {
			TRACE(cout << "set " << i << io::endl);
			tab->set(j++, reg);
		}
	}
	switch(inst->ident) {
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:
		TRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		break;
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
		TRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[2].val.uint8) / 4]);
		assert(tab->get(j - 1));
	case ID_MCRF_CRF_CRF:
		TRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[1].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;
	}
	reads = tab;
	assert(j == cnt);

	// Count the write registers
	cnt = 0;
	for(i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instroutput + i)) {
			TRACE(cout << "count " << i << io::endl);
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
		TRACE(cout << "count " << i << io::endl);
		cnt++;
		break;		
	}
	tab = new elm::genstruct::AllocatedTable<hard::Register *>(cnt);

	// Get write registers
	#ifdef SCAN_ARGS
		cout << "INSTROUTPUT\n" << cnt << ": ";
		scan_args.decodeParams(cout, inst->instroutput);
		cout << '\n';
	#endif
	for(i = 0, j = 0; inst->instroutput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instroutput + i);
		if(reg) {
			TRACE(cout << "set " << i << io::endl);
			tab->set(j++, reg);
		}
	}
	switch(inst->ident) {
	case ID_CMPI_R_: case ID_CMP_R_R: case ID_CMPLI_R_: case ID_CMPL_R_R:
	case ID_FCMPU_CRF_FR_FR: case ID_FCMPO_CRF_FR_FR:
	case ID_MCRXR_CRF: case ID_MCRFS_CRF_CRF:
		TRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;		
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
	case ID_MCRF_CRF_CRF:
		TRACE(cout << "set " << i << io::endl);
		tab->set(j++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		assert(tab->get(j - 1));
		break;
	}
	writes = tab;
	assert(j == cnt);
	
	// Free instruction
	iss_free(inst);
}

void Inst::scanCustom(instruction_t *inst) {
	flags &= ~(IS_CONTROL | IS_CALL | IS_RETURN);
}


/**
 */
Inst::kind_t Inst::kind(void) {
	scan();
	return flags & ~BUILT;
}

} } // otawa::gliss
