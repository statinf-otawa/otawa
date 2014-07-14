/*
 *	dcache::CLPBlockBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/Memory.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Process.h>
#include <otawa/dcache/CLPBlockBuilder.h>
#include <otawa/data/clp/features.h>

namespace otawa { namespace dcache {

/**
 * @class CLPBlockBuilder
 * Build the list of blocks used for L1 data cache analysis and decorate each basic block
 * with the list of performed accesses, based on a CLP analysis
 *
 * @p Provided Features
 * @li @ref DATA_BLOCK_FEATURE
 *
 * @p Required Features
 * @li @ref
 *
 * @p Configuration
 * @li @ref INITIAL_SP
 * @ingroup dcache
 */
p::declare CLPBlockBuilder::reg = p::init("otawa::dcache::CLPBlockBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<CLPBlockBuilder>()
	.provide(DATA_BLOCK_FEATURE)
	.require(otawa::clp::FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);


/**
 */
CLPBlockBuilder::CLPBlockBuilder(p::declare& r): BBProcessor(r), mem(0), cache(0), colls(0), man(0) {
}


/**
 */
void CLPBlockBuilder::setup(WorkSpace *ws) {

	// get cache
	cache = hard::CACHE_CONFIGURATION(ws)->dataCache();
	if(!cache)
		throw otawa::Exception("no data cache !");
	if(cache->replacementPolicy() != hard::Cache::LRU)
		throw otawa::Exception("unsupported replacement policy in data cache !");

	// get memory
	mem = &ws->process()->platform()->memory();

	// build the block collection
	colls = new BlockCollection[cache->rowCount()];
	DATA_BLOCK_COLLECTION(ws) = colls;
	for(int i = 0; i < cache->rowCount(); i++)
		colls[i].setSet(i);

	// allocate the manager
	man = new clp::Manager(ws);
}


/**
 */
void CLPBlockBuilder::cleanup(WorkSpace *ws) {
	if(man)
		delete man;
}


/**
 */
void CLPBlockBuilder::processBB (WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;
	clp::Manager::step_t step = man->start(bb);
	genstruct::Vector<Pair<clp::Value, BlockAccess::action_t> > addrs;
	while(step) {

		// scan the instruction
		sem::inst i = man->sem();
		BlockAccess::action_t action = BlockAccess::NONE;
		switch(i.op) {
		case sem::LOAD:
			action = BlockAccess::LOAD;
			break;
		case sem::STORE:
			action = BlockAccess::STORE;
			break;
		}

		// add the access
		if(action) {
			clp::Value addr = man->state()->get(clp::Value(clp::REG, i.addr()));
			while(addrs.length() <= man->ipc())
				addrs.push(pair(clp::Value::none, BlockAccess::NONE));
			addr.join(addrs[man->ipc()].fst);
			addrs[man->ipc()] = pair(addr, action);
			if(logFor(LOG_INST))
				log << "\t\t\t" << man->inst()->address() << ": " << man->ipc() << ": " << addrs[man->ipc()] << io::endl;
		}

		// next step
		Inst *inst = man->inst();
		step = man->next();
		if(addrs && (!step || clp::Manager::newInst(step))) {
			for(int i = 0; i < addrs.length(); i++) {
				Pair<clp::Value, BlockAccess::action_t> p = addrs[i];
				if(p.snd) {
					if(p.fst == clp::Value::all)
						accs.add(BlockAccess(inst, p.snd));
					else if(p.fst.isConst()) {
						clp::uintn_t l = p.fst.lower();
						const hard::Bank *bank = mem->get(l);
						if(!bank)
							throw otawa::Exception(_ << "no memory bank for address " << Address(l)
									<< " accessed from " << man->inst()->address());
						const Block& block = colls[cache->set(l)].obtain(cache->round(l));
						accs.add(BlockAccess(inst, p.snd, block));
					}
					else {
						bool over;
						t::uint32 m = elm::mult(elm::abs(p.fst.delta()), p.fst.mtimes(), over);
						// (l % b) + d * n > (R - 1)b		/ R = row count, b = block size
						if(over || m >= (cache->rowCount() - 1) * cache->blockSize() - cache->offset(p.fst.lower()))
							accs.add(BlockAccess(inst, p.snd));
						else {
							clp::uintn_t l = p.fst.start(), h = p.fst.stop();
							const hard::Bank *bank = mem->get(l);
							if(!bank)
								throw otawa::Exception(_ << "no memory bank for address " << Address(l)
										<< " accessed from " << man->inst()->address());
							else if(cache->block(l) == cache->block(h)) {
								const Block& block = colls[cache->set(l)].obtain(l);
								accs.add(BlockAccess(inst, p.snd, block));
							}
							else
								accs.add(BlockAccess(inst, p.snd, cache->set(l), cache->set(h)));
						}
					}
				}
			}
			addrs.clear();
		}

	}

	// record the accesses
	BlockAccess *tab = new BlockAccess[accs.length()];
	for(int i = 0; i < accs.count(); i++) {
		tab[i] = accs[i];
		if(logFor(LOG_BB))
			log << "\t\t\t" << tab[i] << io::endl;
	}
	DATA_BLOCKS(bb) = pair(accs.count(), tab);
	accs.clear();
}

} }	// otawa::dcache
