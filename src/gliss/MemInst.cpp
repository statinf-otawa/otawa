/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	gliss/MemInst.cpp -- gliss::MemInst class implementation.
 */

#include <assert.h>
#include <otawa/gliss/MemInst.h>

namespace otawa { namespace gliss {

/**
 * @class MemInst
 * This class describes an instruction performing memory access.
 */


/**
 */
/*void MemInst::scanCustom(instruction_t *inst) {
	address_t target_addr = 0;
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
		flags |= FLAG_Load;
		break;

	case ID_STFDUX_FR_R_R: case ID_STFDU_FR_R_: case ID_STFDX_FR_R_R:
	case ID_STFD_FR_R_: case ID_STFSUX_FR_R_R: case ID_STFSU_FR_R_:
	case ID_STFSX_FR_R_R: case ID_STFS_FR_R_: case ID_STBUX_R_R_R:
	case ID_STBU_R_R_: case ID_STBX_R_R_R: case ID_STB_R_R_:
	case ID_STHUX_R_R_R: case ID_STHU_R_R_: case ID_STHX_R_R_R:
	case ID_STH_R_R_: case ID_STWUX_R_R_R: case ID_STWU_R_R_:
	case ID_STWX_R_R_R: case ID_STW_R_R_: case ID_STWBRX_R_R_R:
	case ID_STHBRX_R_R_R: case ID_STMW_R_R_: case ID_STSWX_R_R_R:
	case ID_STSWI_R_R_:
		flags |= FLAG_Store;
		break;
	
	default:
		assert(0);
	}
}*/


/**
 * @fn MemInst::MemInst(CodeSegment& segment, address_t address); 
 * Build a new memory instruction.
 * @param segment	Segment containing the instruction.
 * @param address	Instruction address.
 */


/**
 */
/*bool MemInst::isMem(void) {
	return true;
}*/


/**
 */
/*bool MemInst::isLoad(void) {
	scan();
	return flags & FLAG_Load;
}*/


/**
 */
/*bool MemInst::isStore(void) {
	scan();
	return flags & FLAG_Store;
}*/

} } // otawa::gliss
