/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/VirtualBasicBlock.cpp -- implementation of VirtualBasicBlock class.
 */

#include <otawa/cfg/VirtualBasicBlock.h>

namespace otawa {

/**
 * Build a new virtual basic block.
 * @param bb	Virtualized basic block.
 */
VirtualBasicBlock::VirtualBasicBlock(BasicBlock *bb): _bb(bb) {

	// Find root BB
	while(_bb->isVirtual())
		_bb = ((VirtualBasicBlock *)_bb)->_bb;

	// Initialization
	flags = _bb->getFlags() | FLAG_Virtual;
	first = bb->firstInst();
	_size = bb->size();
	addProps(*bb);
};

} // otawa
