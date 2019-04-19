/*
 *	icat3::LBlockBuilder class implementation
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

#include "../../include/otawa/icat3/features.h"
#include <elm/data/HashMap.h>
#include <elm/ptr.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/icache/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/EdgeProcessor.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace icat3 {

/**
 * @class LBlock
 * An l-block is a instruction cache block matchin some code
 * in the current program.
 */

LBlock::LBlock(Address address, int index, int set): _address(address), _index(index), _set(set) {
}


/**
 * @class LBlockCollection
 * List of l-blocks used by the program sorted by set. Provides also the
 * instruction cache to which the block structure applies.
 */

/**
 * Build a l-block collection.
 * @param sets		Number of sets.
 * @param cache		Current instruction cache.
 */
LBlockCollection::LBlockCollection(int sets, const hard::Cache *cache)
	: Bag<LBlockSet>(sets), _cache(cache) { }


/**
 */
LBlockSet::~LBlockSet(void) {
	for(int i = 0; i < size(); i++)
		delete get(i);
}

/**
 */
class LBlockBuilder: public Processor {
public:
	static p::declare reg;
	LBlockBuilder(void): Processor(reg) { }

protected:

	typedef HashMap<Address, LBlock *> map_t;
	typedef Vector<Vector<LBlock *> > lblocks_t;

	virtual void processWorkSpace(WorkSpace *ws) {

		// get the cache
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION_FEATURE.get(ws);
		ASSERT(conf);
		const hard::Cache *cache = conf->instCache();
		if(!cache)
			throw ProcessorException(*this, "no instruction cache!");
		if(cache == conf->dataCache())
			throw ProcessorException(*this, "unified cache not supported!");
		UniquePtr<hard::Cache> to_free;
		switch(cache->replacementPolicy()) {
		case hard::Cache::LRU:
			break;
		case hard::Cache::RANDOM:		// considered as LRU cache with A=1
			to_free = new hard::Cache(*cache);
			to_free->setWayBits(0);
			cache = to_free;
			break;
		default:
			if(cache->wayCount() > 1)	// other caches with A=1 considered as direct-mapped
				throw ProcessorException(*this, "only LRU caches are supported!");
		}

		// build the blocks
		LBlockCollection *coll = new LBlockCollection(cache->setCount(), cache);
		lblocks_t vecs(cache->setCount());
		vecs.setLength(cache->setCount());
		track(LBLOCKS_FEATURE, LBLOCKS(ws) = coll);
		map_t map;

		// traverse the program
		const CFGCollection *cfgs = INVOLVED_CFGS(ws);
		ASSERT(cfgs);
		for(CFGCollection::Iter cfg(cfgs); cfg(); cfg++) {
			if(logFor(LOG_FUN))
				log << "\tfunction " << *cfg << io::endl;
			for(CFG::BlockIter b = cfg->blocks(); b(); b++) {
				if(b->hasProp(icache::ACCESSES))
					collect(map, *icache::ACCESSES(*b), vecs, cache);
				for(Block::EdgeIter e = b->outs(); e(); e++)
					if(e->hasProp(icache::ACCESSES))
						collect(map, *icache::ACCESSES(*e), vecs, cache);
			}
		}

		// finalize the collection
		for(int i = 0; i < cache->setCount(); i++)
			(*coll)[i] << vecs[i];
	}

	void collect(map_t& map, Bag<icache::Access>& bag, lblocks_t& vecs, const hard::Cache *cache) {
		for(int i = 0; i < bag.count(); i++)
			switch(bag[i].kind()) {
			case icache::NONE:
				break;
			case icache::FETCH:
			case icache::PREFETCH: {
					Address a = cache->round(bag[i].address());
					LBlock *lb = map.get(a, 0);
					if(!lb) {
						int set = cache->set(a);
						lb = new LBlock(a, vecs[set].count(), set);
						vecs[set].add(lb);
						map.put(a, lb);
						if(logFor(LOG_BLOCK))
							log << "\t\tl-block " << lb->index() << " at " << lb->address() << ", set" << set << io::endl;
					}
					LBLOCK(bag[i]) = lb; // associated the cache access with the corresponding LBlock
				}
				break;
			default:
				ASSERT(false);
			}
	}

};

p::declare LBlockBuilder::reg = p::init("otawa::icat3::LBlockBuilder", Version(1, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(icache::ACCESSES_FEATURE)
	.provide(LBLOCKS_FEATURE)
	.maker<LBlockBuilder>();


/**
 *  This feature ensures that the instruction cache accesses has been marked with L-Block
 *  objects which are stored in a collection of all L-Blocks of the current code. It also
 *  ensures that the cache is analyzable by icat3 module.
 *
 *  @par Default Processor
 *  @li @ref LBlockBuilder
 *
 *  @par Properties
 *  @li @ref LBLOCKS (on @ref WorkSpace)
 *  @li @ref LBLOCK (on @ref icache::Access)
 */
p::feature LBLOCKS_FEATURE("otawa::icat3::LBLOCKS_FEATURE", LBlockBuilder::reg);


/**
 * Provide the list of used L-Blocks.
 *
 * @par Hook
 * @li @ref WorkSpace
 *
 * @par Feature
 * @li @ref LBLOCKS_FEATURE
 */
p::id<LBlockCollection *> LBLOCKS("otawa::icat3::LBLOCKS", 0);


/**
 * Provide the L-Block used by an instruction cache access.
 *
 * @par Hook
 * @li @ref Access
 *
 * @par Feature
 * @li @ref LBLOCKS_FEATURE
 */
p::id<LBlock *> LBLOCK("otawa::icat3::LBLOCK", 0);

} }		// otawa::icat3
