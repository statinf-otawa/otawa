/*
 *	LBlock interface
 *	Copyright (c) 2005-17, IRIT UPS <casse@irit.fr>
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
#include <elm/data/List.h>
#include <elm/data/HashMap.h>
#include <elm/inhstruct/DLList.h>
#include <elm/PreIterator.h>
#include <otawa/instruction.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/hard/Cache.h>

namespace otawa { namespace cache {

// Extern classes
class LBlockSet;

// LBlock class
class LBlock: public elm::inhstruct::DLNode, public PropList {
	friend class LBlockSet;
public:
	LBlock(LBlockSet *set, BasicBlock *bb, Inst *inst, t::uint32 size, int cache_index, address_t addr);
	inline int index(void) const { return idx; }
	inline Address address(void) const { return _addr; }
	inline BasicBlock *bb(void) const { return _bb; }
	inline ot::size size(void) const { return _size; }
	inline LBlockSet *lblockset(void) const { return lbs; }
	int countInsts(void);
	inline int cacheBlock(void) const { return cid - 1; }
	inline Inst *instruction(void) const { return _inst; }
	
	// deprecated
	inline int number(void) const { return idx; }
	inline int id(void) { return idx; }
	inline int cacheblock(void) const { return cid; }

private:
	~LBlock(void);
	LBlockSet *lbs;
	Inst *_inst;
	t::uint32 _size;
	BasicBlock *_bb;
	int idx, cid;
	address_t _addr;
};

Output& operator<<(Output& out, const LBlock *block);

} } // otawa::cache

#endif // OTAWA_CACHE_LBLOCK_H
