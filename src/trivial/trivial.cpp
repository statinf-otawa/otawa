/*
 *	trivial module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/cache/features.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Memory.h>
#include <otawa/icache/features.h>
#include <otawa/ilp/expr.h>
#include <otawa/ipet.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/stats/BBStatCollector.h>
#include <otawa/trivial/features.h>

namespace otawa { namespace trivial {

using namespace ilp;

/**
 * @defgroup trivial	Trivial Analyzes
 *
 * This plugin contains several trivial analyzes. They do not provide
 * a realistic WCET computation but they may be used for different purposes:
 * @li to have a fast WCET computation environment in order to test a specific analysis,
 * @li to overestimate some effects hardware effects to obtain a maximum overestimation,
 * @li to get coarse-grain but fast WCET computation.
 *
 * This plugin contains the following analyzes:
 * @li @ref  BlockTime -- computes block time considering a fix time for each instruction (default to 5 cycles).
 * @li @ref AllMissICacheTime -- consider all instruction cache accesses as misses and add this time to the WCET objective function.
 * @li @ref NoDCache -- consider there is no data cache or all data cache accesses results in misses.
 */

/**
 * Compute the time of blocks considering that each instruction takes the same amount of time (default to 5 cycles)
 * or as defined by the configuration property @ref INSTRUCTION_TIME.
 *
 * Required features:
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 *
 * Provided features:
 * @li @ref ipet::BB_TIME_FEATURE
 *
 * Configuration properties:
 * @li @ref INSTRUCTION_TIME
 *
 * @ingroup trivial
 */
class BlockTime: public BBProcessor {
public:
	static p::declare reg;

	BlockTime(p::declare& r = reg): BBProcessor(r), itime(5) { }

	virtual void configure(const PropList& props) {
		BBProcessor::configure(props);
		itime = INSTRUCTION_TIME(props);
	}

protected:
	// virtual void processBB(WorkSpace *fw, CFG *cfg, Block *bb) {
	// 	if(!bb->isBasic())
	// 		ipet::TIME(bb) = 0;
	// 	else
	// 		ipet::TIME(bb) = itime * bb->toBasic()->count();
	// }
	virtual void processBB(WorkSpace *fw, CFG *cfg, Block *b) override {
		if(!b->isBasic())
			ipet::TIME(b) = 0;
		else {
			BasicBlock *bb = b->toBasic();
			unsigned bb_cycles = 0;
			for(BasicBlock::InstIter insts = bb->begin(); insts != bb->end(); insts++) {
				const auto& inst = insts.item();
				Option<unsigned> c = inst->cycles();
				if(!c) {
					cerr << "WARNING: instruction ";
					inst->dump(cerr);
					cerr << ": cycles() undefined, using 8\n";
					c = 8;
				}
				bb_cycles += *c;
			}
			ipet::TIME(b) = bb_cycles;
		}
	}

	virtual void collectStats(WorkSpace *ws) {
		record(new ipet::TimeStat(ws));
	}

	virtual void destroy(WorkSpace *ws, CFG *cfg, Block *b) {
		ipet::TIME(b).remove();
	}

private:
	int itime;
};

/**
 */
p::declare BlockTime::reg = p::init("otawa::trivial::BlockTime", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<BlockTime>()
	.provide(ipet::BB_TIME_FEATURE);

/**
 * This configuration property provides the time of an instruction in cycles.
 *
 * User Analyzes or Features:
 * @li @ref BlockTime
 */
p::id<int> INSTRUCTION_TIME("otawa::trivial::INSTRUCTION_TIME", 5);


/**
 * This analysis considers that all instruction cache accesses are misses and increases the WCET
 * objective function with the cache access time.
 *
 * Provided features:
 * @li @ref ipet::INST_CACHE_SUPPORT_FEATURE
 *
 * Required features:
 * @li @ref cache::COLLECTED_LBLOCKS_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref hard::MEMORY_FEATURE
 * @li @ref hard::CACHE_CONFIGURATION_FEATURE
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 *
 * @ingroup trivial
 */
class AllMissICacheTime: public Processor {
public:
	static p::declare reg;

	AllMissICacheTime(p::declare& r = reg): Processor(r), cache(0), sys(0), mem(0) { }

protected:

	virtual void setup(WorkSpace *ws) {
		cache = hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache();
		if(!cache)
			throw ProcessorException(*this, "no instruction cache available");
		sys = model(ipet::SYSTEM(ws));
		ASSERT(sys);
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		LBlockSet **lblocks = LBLOCKS(ws);
		ASSERT(lblocks);
		for(int i = 0; i < cache->setCount(); i++)
			for(LBlockSet::Iterator lb(*lblocks[i]); lb(); lb++)
				if(lb->bb()) {
					var x(ipet::VAR(lb->bb()));
					ot::time miss_t = mem->readTime(lb->address());
					sys += x * miss_t;
					if(logFor(LOG_INST))
						log << "\tadded miss for " << lb->address() << " (time = " << miss_t << ")\n";
				}
	}

private:
	const hard::Cache *cache;
	model sys;
	const hard::Memory *mem;
};

p::declare AllMissICacheTime::reg = p::init("otawa::trivial::AllMissICacheTime", Version(1, 0, 0))
	.make<AllMissICacheTime>()
	.require(cache::COLLECTED_LBLOCKS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.provide(ipet::INST_CACHE_SUPPORT_FEATURE);



/**
 * This analysis considers that (a) all data cache accesses are misses or (b) there is no data cache.
 * The WCET objective function is increased with the memory access time of load and store instructions.
 *
 * Provided features:
 * @li @ref ipet::DATA_CACHE_SUPPORT_FEATURE
 *
 * Required features:
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref	hard::MEMORY_FEATURE
 *
 * Configuration:
 * @li @ref BLOCKING_STORE -- are stores blocking the pipeline or not.
 *
 * @ingroup trivial
 */
class NoDCacheTime: public BBProcessor {
public:
	static p::declare reg;

	NoDCacheTime(p::declare& r = reg): BBProcessor(r), sys(0), mem(0), blocking_store(true) { }

	virtual void configure(const PropList& props) {
		BBProcessor::configure(props);
		blocking_store = BLOCKING_STORE(props);
	}

protected:

	virtual void setup(WorkSpace *ws) {
		sys = model(ipet::SYSTEM(ws));
		ASSERT(sys);
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();

		// compute the time spent in memory accesses
		ot::time time = 0;
		for(BasicBlock::InstIter i = bb->insts(); i(); i++)
			if(i->isMem()) {
				if(i->isMulti()) {
					if(i->isLoad() && i->isStore() && blocking_store)
						time += mem->worstAccessTime() * i->multiCount();
					else if(i->isLoad())
						time += mem->worstReadTime() * i->multiCount();
					else if(!blocking_store)
						time += mem->worstWriteTime() * i->multiCount();
				}
				else {
					if(i->isLoad())
						time += mem->worstReadTime();
					else if(!blocking_store)
						time += mem->worstWriteTime();
				}
			}

		// add the memory access cost
		if(time) {
			var x(ipet::VAR(b));
			sys += time * x;
		}
	}

private:
	model sys;
	const hard::Memory *mem;
	bool blocking_store;
};

p::declare NoDCacheTime::reg = p::init("otawa::trivial::NoDCacheTime", Version(1, 0, 0))
	.make<NoDCacheTime>()
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(ipet::DATA_CACHE_SUPPORT_FEATURE);

/**
 * This configuration property informs if a store instruction executions blocks the
 * pipeline (default) or not.
 *
 * User analyzes or features:
 * @li @ref NoDCacheTime
 *
 * @ingroup trivial
 */
p::id<bool> BLOCKING_STORE("otawa::trivial::BLOCKING_STORE", true);


/**
 * This analysis uses the category instruction cache analysis to increase
 * the WCET objective function with miss time, considering that a miss causes
 * the full memory access time.
 *
 * @ingroup trivial
 */
class CatICacheTime: public BBProcessor {
public:
	static p::declare reg;
	CatICacheTime(p::declare& r = reg): BBProcessor(r), cache(0), mem(0), sys(0) { }

protected:

	virtual void setup(WorkSpace *ws) {
		cache = hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache();
		if(!cache)
			throw ProcessorException(*this, "no instruction cache here!");
		mem = hard::MEMORY_FEATURE.get(ws);
		sys = model(ipet::SYSTEM(ws));
	}

	void processAccess(PropList *site, const icache::Access& acc) {
		switch(cache::CATEGORY(acc)) {

		case cache::INVALID_CATEGORY:
			warn(_ << "no category at " << acc.address() << " in " << site);
			break;

		case cache::ALWAYS_HIT:
			// no miss penality
			break;

		case cache::FIRST_MISS: {
				ot::time t_miss = mem->readTime(acc.address());
				Block *h = cache::CATEGORY_HEADER(acc);
				if(!h)
					sys += t_miss;
				else {
					for(Block::EdgeIter e = h->ins(); e(); e++)
						if(!otawa::BACK_EDGE(*e))
							sys += t_miss * var(ipet::VAR(*e));
				}
			}
			break;

		case cache::FIRST_HIT:
		case cache::ALWAYS_MISS:
		case cache::NOT_CLASSIFIED: {
				ot::time t_miss = mem->readTime(acc.address());
				var x(ipet::VAR(site));
				sys += t_miss * x;
			}
			break;

		default:
			ASSERT(false);
		}

	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {

		// process accesses of the input edges
		for(Block::EdgeIter e = b->ins(); e(); e++) {
			const Bag<icache::Access>& accs = icache::ACCESSES(*e);
			for(int i = 0; i < accs.count(); i++)
				processAccess(*e, accs[i]);
		}

		// process access of the block
		const Bag<icache::Access>& accs = icache::ACCESSES(b);
		for(int i = 0; i < accs.count(); i++)
			processAccess(b, accs[i]);
	}

private:
	const hard::Cache *cache;
	const hard::Memory *mem;
	model sys;
};

p::declare CatICacheTime::reg = p::init("otawa::trivial::CatICacheTime", Version(1, 0, 0))
	.make<CatICacheTime>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.use(icache::ACCESSES_FEATURE)
	.use(cache::COLLECTED_LBLOCKS_FEATURE)
	.use(otawa::LOOP_INFO_FEATURE)
	.provide(ipet::INST_CACHE_SUPPORT_FEATURE);


/* plugin hook */
ProcessorPlugin plugin = sys::Plugin::make("otawa::trivial", OTAWA_PROC_VERSION)
	.version(Version(1, 0, 0))
	.hook(OTAWA_PROC_NAME);
ELM_PLUGIN(plugin, OTAWA_PROC_HOOK);

} }		// otawa::trivial

