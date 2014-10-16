/*
 *	LBlockSet implementation
 *	Copyright (c) 2005-12, IRIT UPS.
 *
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
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#include <otawa/cache/LBlockSet.h>

namespace otawa {

/**
 * This property is used for storing the list of L-Blocks. The type of its
 * date is LBlockSet *[] with a size equal to the line count of the instruction
 * cache.
 *
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<LBlockSet **> LBLOCKS("otawa::LBLOCKS", 0);

/**
 * This property is used for storing the list of L-Blocks of a BasicBlock.
 * cache.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<genstruct::AllocatedTable<LBlock* >* > BB_LBLOCKS("otawa::BB_LBLOCKS", 0);


/**
 * @class LBlockSet
 * This class represents the list of l-blocks of a task for a chosen cache row.
 * It is useful to perform analysis on l-blocks. It is accessible through
 * the property @ref otawa::LBLOCKS.
 */


/**
 * Add a new l-block.
 * @param lblock	l-block to add.
 */
int LBlockSet::add(LBlock *lblock){
	int id = listelbc.length();
	listelbc.add(lblock);
	if(lblock->cacheBlock() >= cblock_count)
		cblock_count = lblock->cacheBlock() + 1;
	return id;
}


/**
 * Build a l-block set.
 * @param line	Cache row of the l-block set.
 */
LBlockSet::LBlockSet(int line, const hard::Cache *cache): linenumber(line), cblock_count(0), _cache(cache) {
	ASSERT(line >= 0);
}


/**
 * Get a number for a new l-block in the set.
 * Used internally to build l-blocks.
 * @return	New l-block number.
 */
/*int LBlockSet::newCacheBlockID(void) {
	_cacheBlockCount++;
	return(_cacheBlockCount-1);
}*/


/**
 * @fn int LBlockSet::count(void);
 * Get the count of l-blocks in the set.
 * @return	Count of l-blocks.
 */


/**
 * @fn int LBlockSet::cacheBlockCount(void);
 * Get the internal counter of l-block number
 * (only used internally).
 */


/**
 * @fn LBlock *LBlockSet::lblock(int i);
 * Get the l-block with the given index.
 * @param i		Index of the looked block.
 * @return		L-block matching the index.
 */


/**
 * @fn int LBlockSet::set(void);
 * Get the set number of the l-blocks in the cache.
 * @return	Cache set number.
 */

} // otawa
