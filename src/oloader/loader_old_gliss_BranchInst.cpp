/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	loader::old_gliss::BranchInst class implementation
 */

#include <otawa/loader/old_gliss/BranchInst.h>

namespace otawa { namespace loader { namespace old_gliss {

/**
 * @class BranchInst
 * This class provides the default behaviour for a branch instruction, that is,
 * automatic resolution of the target.
 */


/**
 * Build a branch instruction.
 * @param process	Owner process.
 * @param kind		Kind of instruction.
 * @param addr		Address of the instruction.
 */
BranchInst::BranchInst(
	Process& process,
	kind_t kind,
	address_t addr
): Inst(process, kind, addr), _target(0)
{
}


/**
 * This method is called when the target address need to be decoded.
 * @return	Address of the target or null (if it cannot be computed).
 */
address_t BranchInst::decodeTargetAddress(void) {
	return 0;
}


/**
 */
otawa::Inst *BranchInst::target(void) {
	if(!(_kind & TARGET_DONE)) {
		_kind |= TARGET_DONE;
		address_t target_addr = decodeTargetAddress();
		if(_target)
			_target = process().findInstAt(target_addr);
	}
	return _target;
}

} } } // otawa::loader
