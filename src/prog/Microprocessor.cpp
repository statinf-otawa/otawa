/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/exegraph/Microprocessor.cpp -- implementation of Cache class.
 */

#include <otawa/exegraph/Microprocessor.h>
#include <otawa/hard/Register.h>

namespace otawa { 
	
instruction_category_t instCategory(Inst *inst) {
	if ( inst->isControl())
		return CONTROL;
	if (inst->isMem())
		return MEMORY;
	const elm::genstruct::Table<hard::Register *>& writes = inst->writtenRegs();
	for(int i = 0; i < writes.count(); i++) {
		if (writes[i]->kind() == otawa::hard::Register::FLOAT)
			return FALU;
	}
	return IALU;
}

} // otawa