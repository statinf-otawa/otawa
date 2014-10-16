/*
 *	LBlock implementation
 *	Copyright (c) 2005-12, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class interface
 *	This file is part of OTAWA
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <elm/assert.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// TODO		Add for each block set a map of used blocks.

/**
 * @class LBlock
 * The l-block is an abstraction allowing to split the basic block
 * according to the bound of the cache block, that is, instructions composing
 * a l-block are owned by the same basic block and same cache block. Therefore,
 * when the first one is load in the cache, the next instructions are also load.
 */

/**
 * Build a new LBlock.
 * @param lbset			L-block set which owns this l-block.
 * @param inst			Instruction it starts with.
 * @param bb			Basic block containing this l-block.
 * @param size			Size of the l-block.
 * @param cache_indx	Cache index.
 */
LBlock::LBlock(LBlockSet *lbset, BasicBlock *bb, Inst *inst, t::uint32 size, int cache_index)
: lbs(lbset), _inst(inst), _size(size), _bb(bb), cid(cache_index) {
	idx = lbset->LBlockSet::add(this);
}


/**
 * @fn int LBlock::cacheBlock(void) const;
 * Compute the cache block of this L-block.
 * @return	Cache block.
 */


/**
 */
LBlock::~LBlock(void) {
	delete this;
};


/**
 * @fn size_t LBlock::size(void) const;
 * Get the size of the current l-block.
 * @return	L-block size.
 */


/**
 * Count the instructions in the l-block.
 * @return	L-block instruction count.
 */
int LBlock::countInsts(void) {
	int cnt = 0;
	Address addr = _inst->address();
	
	if(_bb != 0)
		for(BasicBlock::InstIter instr(_bb); instr; instr++)
			if(instr->address() >= addr && instr->address() < addr + _size)
				cnt++;
	
	return cnt;	
}


/**
 * @fn int LBlock::number(void) const;
 * Get the number of the LBlock in its cache row.
 * This numbers span from 0 to LBlockSet::count()  - 1.
 * @return	Number of the l-block.
 */


/**
 * @fn int LBlock::id(void) const;
 * @deprecated	Replaced by number().
 */


/**
 * @fn bool LBlock::sameCacheBlock(const LBlock *block) const;
 * Test if both l-blocks belong to the same cache block.
 * @param block		Block to test with.
 * @return			True if they belongs to the same cache block, false else.
 */


/**
 * @fn int LBlock::cacheblock(void);
 * Get a unique number identifying which cache block the l-block belongs to.
 * @param	Cache block number.
 * @deprecated	Currently, there is no way to get back relation with the cache address.
 * For testing if two l-blocks matches the same cache block, uses sameCacheBlock() instead.
 */


/**
 * @fn address_t LBlock::address(void)
 * Get address of the l-block.
 * @return	L-block address.
 */


/**
 * @fn BasicBlock *LBlock::bb(void);
 * Get the BB containing the l-block.
 * @return	Container BB.
 */


/**
 */
Output& operator<<(Output& out, const LBlock *block) {
	out << "lblock(" << block->lblockset()->line() << ", " << block->number() << ")";
	return out;
}

} // otawa
