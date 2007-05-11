/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::new_gliss::BranchInst class interface
 */
#ifndef OTAWA_LOADER_NEW_GLISS_BRANCH_INST_H
#define OTAWA_LOADER_NEW_GLISS_BRANCH_INST_H

#include <otawa/loader/new_gliss/Inst.h>

namespace otawa { namespace loader { namespace new_gliss {

// BranchInst class
class BranchInst: public otawa::loader::new_gliss::Inst {

public:
	BranchInst(Process& process, kind_t kind, address_t addr);
	virtual otawa::Inst *target(void);

protected:
	virtual address_t decodeTargetAddress(void);

private:
	otawa::Inst *_target;
};

} } } // otawa::loader

#endif // OTAWA_LOADER_NEW_GLISS_INST_H
