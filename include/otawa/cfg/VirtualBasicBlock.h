/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cfg/VirtualBasicBlock.h -- interface of VirtualBasicBlock class.
 */
#ifndef OTAWA_CFG_VIRTUAL_BASIC_BLOCK_H
#define OTAWA_CFG_VIRTUAL_BASIC_BLOCK_H

#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// VirtualBasicBlock class
class VirtualBasicBlock: public BasicBlock {
	BasicBlock *_bb;
public:
	VirtualBasicBlock(BasicBlock *bb);
	inline BasicBlock *bb(void) const { return _bb; };
};

} // otawa

#endif	// OTAWA_CFG_VIRTUAL_BASIC_BLOCK_H
