/*
 *	dcache::BlockBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_DCACHE_BLOCKBUILDER_H_
#define OTAWA_DCACHE_BLOCKBUILDER_H_

#include "features.h"
#include <otawa/prog/Inst.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

namespace hard {
	class Cache;
	class Memory;
}

namespace clp { class Manager; }

namespace dcache {

// BlockBuilder class
class BlockBuilder: public BBProcessor {
public:
	static p::declare reg;
	BlockBuilder(p::declare& r = reg);
	virtual void configure(const PropList &props);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, otawa::Block *bb);

private:
	const hard::Cache *cache;
	const hard::Memory *mem;
	Vector<BlockAccess> blocks;
	Address sp;
	BlockCollection *colls;
	clp::Manager *man;
};

} } // otawa

#endif /* OTAWA_DCACHE_BLOCKBUILDER_H_ */
