/*
 *	$Id$
 *	Copyright (c) 2004, IRIT UPS.
 *
 *	src/gliss/Inst.cpp -- GLISS ControlInst class implementation.
 */

#include <otawa/gliss.h>
#include <elm/io.h>

namespace otawa { namespace gliss {

/**
 * @class ControlInst
 * GLISS implementation of the control instruction interface.
 */

/**
 * @fn ControlInst::ControlInst(CodeSegment& segment, address_t address)
 * Build a new control instruction.
 * @param segment	Container segment.
 * @param address	Instruction address.
 */

// Overloaded	
/*bool ControlInst::isConditional(void) {
	scan();
	return flags & FLAG_Cond;
}*/

// Overloaded
otawa::Inst *ControlInst::target(void) {
	scan();
	return _target;
}

// Overload
void ControlInst::scanCustom(instruction_t *inst) {

	// Explore the instruction
	address_t target_addr = 0;
	switch(inst->ident) {
	case ID_BL_:
		flags |= IS_CALL;
	case ID_B_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = address() + (inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BLA_:
		flags |= IS_CALL;
	case ID_BA_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = (address_t)(inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BCL_:
		flags |= IS_CALL;
	case ID_BC_:
		flags |= IS_COND;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = address() + (inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_BCLA_:
		flags |= IS_CALL;
	case ID_BCA_:
		flags |= IS_COND;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = (address_t)(inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_SC:
		break;
	case ID_BCLR_:
		assert(inst->instrinput[0].type == PARAM_UINT5_T);
		assert(inst->instrinput[1].type == PARAM_UINT5_T);
		if(inst->instrinput[0].val.Uint5 == 20
		&& inst->instrinput[1].val.Uint5 == 0) {
			flags |= IS_RETURN;
			break;
		}
	case ID_BCLRL_:
		flags |= (IS_RETURN | IS_COND);
		break;
	case ID_BCCTRL_:
		flags |= IS_CALL;
	case ID_BCCTR_:
	cond:
		flags |= IS_COND;
		break;
	}
	
	// Compute the target if address available
	if(target_addr)
		_target = (Inst *)segment().findInstAt(target_addr);
}

// Overloaded
/*bool ControlInst::isControl(void) {
	return true;
}*/

// Overloaded
/*bool ControlInst::isBranch(void) {
	scan();
	return !(flags & (FLAG_Call | FLAG_Return));
}*/

// Overloaded
/*bool ControlInst::isCall(void) {
	scan();
	return flags & FLAG_Call;
}*/

// Overloaded
/*bool ControlInst::isReturn(void) {
	scan();
	return flags & FLAG_Return;
}*/

// Overloaded
void ControlInst::dump(io::Output& out) {
	
	// Call usual disassembling
	Inst::dump(out);
	
	// Output the branch label if any
	scan();
	if(_target) {
		String label = LABEL(_target);
		if(label)
			out << " [" << label << ']';
	}
}

} } // otawa::gliss
