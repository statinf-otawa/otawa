/*
 *	$Id$
 *	Copyright (c) 2009, IRIT UPS <casse@irit.fr>
 *
 *	MemBlockMap class interface
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

#ifndef OTAWA_UTIL_MEMBLOCKMAP_H_
#define OTAWA_UTIL_MEMBLOCKMAP_H_

#include <otawa/base.h>

namespace otawa {

ot::size leastGreaterLog2(ot::size x);

// MemBlockMap class
template <class T>
class MemBlockMap {

	typedef struct node_t {
		inline node_t(const T *b, node_t *n): next(n), block(b) { }
		struct node_t *next;
		const T *block;
	} node_t;

public:
	MemBlockMap(ot::size block_size = 32, t::size table_size = 211)
	: pow(leastGreaterLog2(block_size)), bsize(1 << pow), tsize(table_size) {
		nodes = new node_t *[tsize];
		for(ot::size i = 0; i < tsize; i++) nodes[i] = 0;
	}

	inline ~MemBlockMap(void) { clear(); delete [] nodes; }

	void clear(void) {
		for(ot::size i = 0; i < tsize; i++) {
			for(node_t *cur = nodes[i], *next; cur; cur = next) {
				next = cur->next;
				delete cur;
			}
			nodes[i] = 0;
		}
	}

	void add(const T *block) {
		ASSERT(block->size());
		ot::size top = (block->topAddress().offset() - 1) >> pow;
		if(top & (bsize - 1))
			top++;
		for(ot::size ptr = block->address().offset() >> pow; ptr <= top; ptr++) {
			ot::size entry = ptr % tsize;
			nodes[entry] = new node_t(block, nodes[entry]);
		}
	}

	const T *get(Address address) {
		ot::size entry = (address.offset() >> pow) % tsize;
		for(node_t *node = nodes[entry]; node; node = node->next)
			if(address >= node->block->address() && address < node->block->topAddress())
				return node->block;
		return 0;
	}

private:
	ot::size pow, bsize, tsize;
	node_t **nodes;
};

}	// otawa

#endif /* OTAWA_UTIL_MEMBLOCKMAP_H_ */
