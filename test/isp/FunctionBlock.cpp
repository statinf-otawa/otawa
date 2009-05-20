/*
 *	$Id$
 *	Copyright (c) 2009, IRIT UPS.
 *
 *	FunctionBlock.cpp -- implementation of FunctionBlock class.
 */

#include <assert.h>
#include "FunctionBlock.h"
#include <otawa/cfg/BasicBlock.h>

namespace otawa {


/**
 * Build a new FunctionBlock.
 * @param bb		Basic block starting this function_block.
 * @param size		Size of the function_block.
 */
  FunctionBlock::FunctionBlock(CFG *cfg)
    : _cfg(cfg) {
}


/**
 * @fn CFG * FunctionBlock::cfg(void) const;
 * Get the cfg of the function related to the block.
 * @return	function-block cfg.
 */


} // otawa
