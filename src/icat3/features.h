/*
 *	features of icat3
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef ICAT3_CMAKEFILES_FEATURES_H_
#define ICAT3_CMAKEFILES_FEATURES_H_

#include <elm/genstruct/Table.h>
#include <otawa/hard/Cache.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace icat3 {

using namespace elm;

// LBlocks
class LBlock {
public:
	LBlock(void): _index(-1) { }
	LBlock(Address address, int index);
	inline Address address(void) const { return _address; }
	inline int index(void) const { return _index; }

private:
	Address _address;
	int _index;
};

class LBlockCollection: public Bag< Bag<LBlock> > {
public:
	LBlockCollection(int sets, const hard::Cache *cache);
	inline const hard::Cache *cache(void) const { return _cache; }
private:
	const hard::Cache *_cache;
};

extern p::feature LBLOCKS_FEATURE;
extern p::id<LBlockCollection *> LBLOCKS;
extern p::id<LBlock *> LBLOCK;

} }		// otawa::icat3

#endif /* ICAT3_CMAKEFILES_FEATURES_H_ */
