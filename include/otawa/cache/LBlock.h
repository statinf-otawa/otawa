/*
 *	LBlock interface
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
#ifndef OTAWA_CACHE_LBLOCK_H
#define OTAWA_CACHE_LBLOCK_H

#include <elm/string.h>
#include <elm/genstruct/SLList.h>
#include <elm/genstruct/HashTable.h>
#include <elm/inhstruct/DLList.h>
#include <elm/PreIterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/hard/Cache.h>

namespace otawa {

// Extern classes
class LBlockSet;

// LBlock class
class LBlock: public elm::inhstruct::DLNode, public PropList {
	friend class LBlockSet;
public:
	LBlock(LBlockSet *graphe, address_t head, BasicBlock *bb, t::uint32 size, int _cacheblock);
	
	inline int number(void) const { return ident; }
	inline address_t address(void) const { return addr; }
	inline BasicBlock *bb(void) const { return _bb; }
	inline t::size size(void) const { return _size; }
	inline bool sameCacheBlock(const LBlock *block) const { return _cacheblock == block->_cacheblock; }
	inline LBlockSet *lblockset(void) const { return lbs; }
	int countInsts(void);

	// deprecated
	inline int id(void) { return ident; }
	inline int cacheblock(void) { return _cacheblock; }

private:
	~LBlock(void);

	LBlockSet *lbs;
	address_t addr;
	t::uint32 _size;
	int ident;
	int _cacheblock;
	BasicBlock *_bb;
};

Output& operator<<(Output& out, const LBlock *block);

} // otawa

#endif // OTAWA_CACHE_LBLOCK_H
