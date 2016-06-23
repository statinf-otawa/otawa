/*
 *	icache features
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

#include <otawa/icache/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Memory.h>

namespace otawa { namespace icache {

using namespace elm;

class AccessBuilder: public BBProcessor {
public:
	static p::declare reg;
	AccessBuilder(void): BBProcessor(reg), icache(0), mem(0) { }

protected:

	virtual void setup(WorkSpace *ws) {
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION(ws);
		ASSERT(conf);
		icache = conf->instCache();
		if(!icache && logFor(LOG_FUN))
			log << "\tno instruction cache available; nothing to do.\n";
		mem = hard::MEMORY(ws);
		ASSERT(mem);
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		if(icache)
			BBProcessor::processWorkSpace(ws);
	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();

		// is it cached (assuming a BB does span over several banks)?
		const hard::Bank *bank = mem->get(bb->address());
		if(!bank->isCached())
			return;

		// split the BB in block accesses
		BasicBlock::InstIter i;
		if(i) {
			accs.add(Access(FETCH, i, i->address()));
			i++;
		}
		for(; i; i++)
			if(icache->offset(i->address()) == 0)
				accs.add(Access(FETCH, i, i->address()));

		// build the property
		ACCESSES(bb) = accs;
		accs.clear();
	}

private:
	const hard::Cache *icache;
	const hard::Memory *mem;
	genstruct::Vector<Access> accs;
};

p::declare AccessBuilder::reg = p::init("otawa::icache::AccessBuilder", Version(1, 0, 0))
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(ACCESSES_FEATURE)
	.make<AccessBuilder>()
	.extend<BBProcessor>();


/**
 * This feature ensures that information about the instruction
 * cache behavior has been added to the CFG using
 * @ref ACCESSES property.
 *
 * @par Properties
 * @li @ref ACCESSES
 */
p::feature ACCESSES_FEATURE("otawa::icache::ACCESSES_FEATURE", p::make<BBProcessor>());



/**
 * This property defines the instruction caches performed by a basic block
 * or when an edge is traversed (mainly to support prefetching existing on some
 * microprocessors).
 *
 * @par Hooks
 * @li @ref BasicBlock -- accessed memories.
 * @li @ref Edge -- to support prefetching before a branch is performed.
 */
p::id<Bag<Access> > ACCESSES("otawa::icache::ACCESSES");

} }	// otawa::icache
