/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::old_gliss::BranchInst class interface
 */
#ifndef OTAWA_LOADER_OLD_GLISS_BRANCH_INST_H
#define OTAWA_LOADER_OLD_GLISS_BRANCH_INST_H

#include <otawa/loader/old_gliss/Inst.h>

namespace otawa { namespace loader { namespace old_gliss {

// BranchInst class
class BranchInst: public otawa::loader::old_gliss::Inst {

public:
	BranchInst(Process& process, kind_t kind, address_t addr);

	virtual otawa::Inst *target(void);

protected:
	virtual address_t decodeTargetAddress(void);

private:
	otawa::Inst *_target;
};

} } } // otawa::loader

#endif // OTAWA_LOADER_OLD_GLISS_INST_H
