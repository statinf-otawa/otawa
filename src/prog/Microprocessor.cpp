/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/exegraph/Microprocessor.cpp -- implementation of Cache class.
 */

#include <otawa/exegraph/Microprocessor.h>
#include <otawa/hard/Register.h>

namespace otawa { 

static const unsigned long MUL_MASK = Inst::IS_MUL | Inst::IS_INT;
static const unsigned long DIV_MASK = Inst::IS_DIV | Inst::IS_INT;
	
instruction_category_t instCategory(Inst *inst) {
	Inst::kind_t kind = inst->kind();
	if(kind & Inst::IS_FLOAT)
		return FALU;
	else if(kind & Inst::IS_MEM)
		return MEMORY;
	else if(kind & Inst::IS_CONTROL)
		return CONTROL;
	else if((kind & MUL_MASK) == MUL_MASK)
		return MUL;
	else if((kind & DIV_MASK) == DIV_MASK)
		return MUL;
	else
		return IALU;
}

} // otawa
