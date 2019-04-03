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
	.provide(CLP_BLOCK_FEATURE)
	.require(otawa::clp::CLP_ANALYSIS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE);


/**
 * This feature ensures that accessed data blocks have been built for data cache analysis based
 * on the reference obtaine from CLP analysis.
 *
 * @par Default Processor
 * @li @ref CLPBlockBuilder
 *
 * @see clp
 * @ingroup dcache
 */
p::feature CLP_BLOCK_FEATURE("otawa::dcache::CLP_BLOCK_FEATURE", new Maker<CLPBlockBuilder>());


/**
 */
CLPBlockBuilder::CLPBlockBuilder(p::declare& r): BBProcessor(r), cache(0), mem(0), colls(0), man(0) {
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
	mem = hard::MEMORY_FEATURE.get(ws);

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
void CLPBlockBuilder::processBB (WorkSpace *ws, CFG *cfg, otawa::Block *b) {
	if(!b->isBasic())
		return;
	BasicBlock *bb = b->toBasic();
	clp::Manager::step_t step = man->start(bb);
	Vector<Pair<clp::Value, BlockAccess::action_t> > addrs;
	while(step) {
		if(man->state()->equals(clp::State::EMPTY)) {
			step = man->next();
			continue;
		}

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
		if(action && (i.memIndex() != 0)) {
			// clp::Value addr = man->state()->get(clp::Value(clp::REG, i.addr())); // if LOAD T1, T1, uint32, then T1 is already over-written
			clp::Value addr = man->getCurrentAccessAddress();
			while(addrs.length() <= man->ipc())
				addrs.push(pair(clp::Value::none, BlockAccess::NONE));
			addr.join(addrs[man->ipc()].fst);
			addrs[man->ipc()] = pair(addr, action);
			if(logFor(LOG_INST))
				log << "\t\t\tMemAccess:" << man->inst()->address() << ": " << man->ipc() << ": " << addrs[man->ipc()] << io::endl;
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
					else if(p.fst.isConst()) { // constant address
						clp::uintn_t l = p.fst.lower(); // the actual address
						const hard::Bank *bank = mem->get(l);
						if(!bank)
							throw otawa::Exception(_ << "no memory bank for address " << Address(l)
									<< " accessed from " << man->inst()->address());
						if(!bank->isCached())
							continue;
						const Block& block = colls[cache->set(l)].obtain(cache->round(l));
						accs.add(BlockAccess(inst, p.snd, block));
					}
					else {
						bool over; // see if the memory range is limited (without overflow)
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
							else if(!bank->isCached())
								continue;
							else if(cache->block(l) == cache->block(h)) { // if all the access addresses are in the same cached block
								const Block& block = colls[cache->set(l)].obtain(cache->round(l));
								accs.add(BlockAccess(inst, p.snd, block));
							}
							else {
								BlockAccess ba = BlockAccess(inst, p.snd, cache->set(l), cache->set(h));
								Address begin = 0, end = 0;
								if(h > l) {
									begin = cache->round(l);
									end = cache->round(h);
								}
								else {
									begin = cache->round(h);
									end = cache->round(l);
								}
								for(Address curr = begin; curr <= end; curr = curr + cache->blockSize()) {
									const Block& block = colls[cache->set(curr)].obtain(cache->round(curr));
									ba.addBlock(&block);
								}
								accs.add(ba);
							}
						}
					}
				}
			}
			addrs.clear();
		} // end of if(addrs && (!step || clp::Manager::newInst(step))) {
	} // end of steps

	// record the accesses
	BlockAccess *tab = new BlockAccess[accs.length()];
	for(int i = 0; i < accs.count(); i++) {
		tab[i] = accs[i];
		if(logFor(LOG_BB))
			log << "\t\t\tBlockAccess:" << tab[i] << io::endl;
	}
	DATA_BLOCKS(bb) = pair(accs.count(), tab);
	accs.clear();
}

} }	// otawa::dcache
