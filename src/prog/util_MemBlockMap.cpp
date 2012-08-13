/*
 *	$Id$
 *	Copyright (c) 2009, IRIT UPS <casse@irit.fr>
 *
 *	MemBlockMap class implementation
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

#include <otawa/util/MemBlockMap.h>

namespace otawa {

/**
 * Compute the least greater logarithm of 2 for the given value.
 * @param x	Value to get LGL2.
 * @return	LGL2 of x.
 */
ot::size leastGreaterLog2(ot::size x) {
	if(x == 0)
		return 0;
	ot::size off = 0, mask = 0xffff, cnt = 16;
	while(cnt != 0) {
		ot::size fmask = mask << (off + cnt);
		if(x & fmask)
			off += cnt;
		cnt >>= 1;
		mask >>= cnt;
	}
	if(x & ((1 << off) - 1))
		off++;
	return off;
}


/**
 * @class MemBlockMap
 * This class is used to retrieve quickly blocks from their containing address.
 * Stored blocks must match the Block concept of OTAWA:
 * @code
 * class Block {
 * public:
 * 		Address address(void) const;
 * 		Address topAddress(void) const;
 * 		size_t size(void) const;
 * };
 * @endcode
 *
 * @param T	Type of stored memory blocks.
 * @author H. Cassé <casse@irit.fr>
 */


/**
 * @fn MemBlockMap::MemBlockMap(size_t block_size, size_t table_size);
 * Build a memory block map.
 * @param block_size	Average size of the stored blocks.
 * @param table_size	Hash table size (better if it is a prime number).
 */


/**
 * @fn void MemBlockMap::clear(void);
 * Clear the content of  map.
 */


/**
 * @fn void MemBlockMap::add(const T& block);
 * Add a block to the map.
 * @param block		Block to add.
 */


/**
 * @fn const T& MemBlockMap::get(Address address);
 * Find a block by its address.
 * @param address	Address of the block to get.
 * @return			Block matching the address or null if none is found.
 */

}	// otawa
