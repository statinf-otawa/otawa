/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS <casse@irit.fr>
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
#ifndef OTAWA_UTIL_LBLOCKBUILDER_H
#define OTAWA_UTIL_LBLOCKBUILDER_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/proc/Feature.h>
#include <elm/genstruct/Vector.h> 
#include <elm/genstruct/Table.h>

namespace otawa {

namespace hard {
	class Cache;
}

// LBlockBuilder class
class LBlockBuilder: public BBProcessor {
	LBlockSet **lbsets;
	const hard::Cache *cache;
	HashTable<int,int> *cacheBlocks;
	//void processLBlockSet(WorkSpace *fw, CFG *cfg, LBlockSet *lbset, const hard::Cache *cach, int *tableindex);
	void addLBlock(
		BasicBlock *bb,
		Inst *inst,
		int& index,
		genstruct::AllocatedTable<LBlock *> *lblocks);

protected:
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
	virtual void cleanup(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);

public:
	LBlockBuilder(void);
};

// Features
extern Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE;

} // otawa

#endif // OTAWA_UTIL_LBLOCKBUILDER_H
