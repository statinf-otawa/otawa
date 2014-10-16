/*
 *	LBlockSet interface
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
#ifndef OTAWA_CACHE_LBLOCKSET_H
#define OTAWA_CACHE_LBLOCKSET_H

#include <elm/assert.h>
#include <otawa/properties.h>
#include <otawa/instruction.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/Table.h>
#include <otawa/cache/LBlock.h>
#include <elm/PreIterator.h>

using namespace elm;

namespace otawa {

// LBlockSet class
class LBlockSet {
	friend class CCGDFA;
public:

	// Iterator class
	class Iterator:  public elm::genstruct::Vector<LBlock *>::Iterator {
	public:
		inline Iterator(LBlockSet& lbset):
			elm::genstruct::Vector<LBlock *>::Iterator(lbset.listelbc) { }
	};
	
	// Methods
	LBlockSet(int row, const hard::Cache *cache);
	int add(LBlock *node);
	inline int count(void) { return listelbc.length(); }
	inline int cacheBlockCount(void) { return listelbc.length(); }
	inline LBlock *lblock(int i) { return listelbc[i]; }
	inline int set(void) { return linenumber; }
	inline const hard::Cache *cache(void) const { return _cache; }

	// deprecated
	int line(void) { return linenumber; }

private:
	int linenumber;
	elm::genstruct::Vector<LBlock *> listelbc;
	int cblock_count;
	const hard::Cache *_cache;
};

// Properties
extern Identifier<LBlockSet **> LBLOCKS;
extern Identifier<genstruct::AllocatedTable<LBlock* >* > BB_LBLOCKS;	 

} //otawa

#endif // OTAWA_CACHE_LBLOCKSET_H
