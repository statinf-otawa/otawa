/*
 *	LBlockBuilder class interface
 *	Copyright (c) 2005-12, IRIT UPS <casse@irit.fr>
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_CACHE_LBLOCKBUILDER_H
#define OTAWA_CACHE_LBLOCKBUILDER_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/proc/Feature.h>
#include <elm/data/Vector.h>
#include <elm/data/Array.h>

namespace otawa {

namespace hard {
	class Cache;
	class Memory;
}

namespace cache {

// LBlockBuilder class
class LBlockBuilder: public BBProcessor {
public:
	static p::declare reg;
	LBlockBuilder(AbstractRegistration& r = reg);

protected:
	virtual void processBB(WorkSpace *fw, CFG *cfg, Block *bb);
	virtual void cleanup(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);

private:
	LBlockSet **lbsets;
	const hard::Cache *cache;
	const hard::Memory *mem;
	HashMap<ot::mask, int> block_map;
	void addLBlock(BasicBlock *bb, Inst *inst, int& index, AllocArray<LBlock *> *lblocks, Address addr);
};

} }	// otawa::cachze

#endif // OTAWA_CACHE_LBLOCKBUILDER_H
