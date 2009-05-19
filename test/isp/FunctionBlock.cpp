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
  FunctionBlock::FunctionBlock(BasicBlock *entry_bb, size_t size)
    : _size(size), _entry_bb(entry_bb) {
}


/**
 * @fn size_t FunctionBlock::size(void) const;
 * Get the size of the function_block.
 * @return	function-block size.
 */



/**
 * @fn BasicBlock *FunctionBlock::entryBB(void);
 * Get the entry BB of the function_block.
 * @return	Entry BB.
 */

} // otawa
