/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/Instruction.cpp -- gliss::Instruction class implementation.
 */

#include <elm/io.h>
#include <otawa/gliss.h>

using namespace elm;

#ifndef NDEBUG
//#	define SCAN_ARGS
#endif

namespace otawa { namespace gliss {


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
		return ids[param->type].type != NO_SUPPORT;
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
 * @class Inst
 * Representation of instructions in Gliss.
 */


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
	
	// Display the hex value
	
	
	// Disassemble the statement
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)addr, buffer);
	inst = iss_decode((::address_t)(unsigned long)addr, buffer);
	iss_disasm(out_buffer, inst);
	out << out_buffer;
	iss_free(inst);
}


/**
 * Scan the instruction for filling the object.
 */
void Inst::scan(void) {

	// Already computed?
	if(flags & FLAG_Built)
		return;
	
	// Get the instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)address(), buffer);
	inst = iss_decode((::address_t)(unsigned long)address(), buffer);
	assert(inst);
	
	// Call customization
	scanCustom(inst);
	
	// Cleanup
	iss_free(inst);
	flags |= FLAG_Built;
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
	inst = iss_decode((::address_t)(unsigned long)addr, buffer);
	
	// Count read registers
	int cnt = 0;
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instrinput + i))
			cnt++;
	switch(inst->ident) {
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
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
	cnt = 0;
	for(int i = 0; inst->instrinput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instrinput + i);
		if(reg)
			tab->set(cnt++, reg);
	}
	switch(inst->ident) {
	case ID_BCLR_: case ID_BCLRL_: case ID_BCCTR_: case ID_BCCTRL_:
	case ID_BCLA_: case ID_BCL_: case ID_BCA_: case ID_BC_:
		tab->set(cnt++, Platform::CR_bank[(31 - inst->instrinput[0].val.uint8) / 4]);
		break;
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
		tab->set(cnt++, Platform::CR_bank[7 - inst->instrinput[2].val.uint8]);
	case ID_MCRF_CRF_CRF:
		tab->set(cnt++, Platform::CR_bank[7 - inst->instrinput[1].val.uint8]);
		break;
	}
	reads = tab;

	// Count the write registers
	cnt = 0;
	for(int i = 0; inst->instroutput[i].type != VOID_T; i++)
		if(scan_args.isSupported(inst->instroutput + i))
			cnt++;
	tab = new elm::genstruct::AllocatedTable<hard::Register *>(cnt);

	// Get write registers
	#ifdef SCAN_ARGS
		cout << "INSTROUTPUT\n" << cnt << ": ";
		scan_args.decodeParams(cout, inst->instroutput);
		cout << '\n';
	#endif
	cnt = 0;
	for(int i = 0; inst->instroutput[i].type != VOID_T; i++) {
		hard::Register *reg = scan_args.reg(inst->instroutput + i);
		if(reg)
			tab->set(cnt++, reg);
	}
	switch(inst->ident) {
	case ID_CMPI_R_: case ID_CMP_R_R: case ID_CMPLI_R_: case ID_CMPL_R_R:
	case ID_FCMPU_CRF_FR_FR: case ID_FCMPO_CRF_FR_FR:
	case ID_MCRXR_CRF: case ID_MCRFS_CRF_CRF:
		tab->set(cnt++, Platform::CR_bank[7 - inst->instrinput[0].val.uint8]);
		break;		
	case ID_CRAND_CRB_CRB_CRB: case ID_CROR_CRB_CRB_CRB:
	case ID_CRXOR_CRB_CRB_CRB: case ID_CRNAND_CRB_CRB_CRB:
	case ID_CRNOR_CRB_CRB_CRB: case ID_CREQV_CRB_CRB_CRB:
	case ID_CRANDC_CRB_CRB_CRB: case ID_CRORC_CRB_CRB_CRB:
	case ID_MCRF_CRF_CRF:
		tab->set(cnt++, Platform::CR_bank[7 - inst->instrinput[0].val.uint8]);
		break;
	}
	writes = tab;
	
	// Free instruction
	iss_free(inst);
}

} } // otawa::gliss
