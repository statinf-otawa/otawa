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
p::declare CLPBlockBuilder::reg = p::init("otawa::dcache::BlockBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<CLPBlockBuilder>()
	.provide(DATA_BLOCK_FEATURE)
	.require(otawa::clp::FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);


/**
 */
CLPBlockBuilder::CLPBlockBuilder(p::declare& r): BBProcessor(r), mem(0), cache(0), colls(0) {
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
}


/**
 */
void CLPBlockBuilder::processBB (WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

	// for each machine instruction

		// for each execution path


	/*AccessedAddresses *addrs = ADDRESSES(bb);
	if(!addrs)
		return;

	// compute block accessed
	for(int i = 0; i < addrs->size(); i++) {
		Address last;
		AccessedAddress *aa = addrs->get(i);

		Address addr;
		switch(aa->kind()) {
		case AccessedAddress::ANY:
			break;
		case AccessedAddress::SP:
			addr = sp + t::uint32(((SPAddress *)aa)->offset());
			break;
		case AccessedAddress::ABS:
			addr = ((AbsAddress *)aa)->address();
			break;
		default:
			ASSERT(false);
		}

		// type of action
		BlockAccess::action_t action = aa->isStore() ? BlockAccess::STORE : BlockAccess::LOAD;

		// access any ?
		if(addr.isNull()) {
			blocks.add(BlockAccess(aa->instruction(), action));
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access any\n";
			continue;
		}

		// is cached ?
		bool cached = false;
		const hard::Bank *bank = mem->get(addr);
		if(!bank)
			throw otawa::Exception(_ << "no memory bank for address " << addr
					<< " accessed from " << aa->instruction()->address());
		else
			cached = bank->isCached();
		if(!cached) {
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access not cached "
					<< addr << "\n";
			continue;
		}

		// create the block access if any
		addr = Address(addr.page(), addr.offset() & ~cache->blockMask());
		int set = cache->line(addr.offset());
		if(last.isNull()) {
			const Block& block = colls[set].obtain(addr);
			blocks.add(BlockAccess(aa->instruction(), action, block));
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access " << addr
					<< " (" << block.index() << ", " << block.set() << ")\n";
			continue;
		}

		// range over the full cache ?
		last = Address(last.page(), (last.offset() - 1) & ~cache->blockMask());
		if(last - addr >= cache->cacheSize()) {
			blocks.add(BlockAccess(aa->instruction(), action));
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access any [" << addr << ", " << last << ")\n";
			continue;
		}

		// a normal range
		int last_set = cache->line(last.offset());
		blocks.add(BlockAccess(aa->instruction(), action, set, last_set));
		if(logFor(LOG_INST))
			log << "\t\t\t\t" << aa->instruction() << " access [" << addr << ", " << last << "] (["
				<< set << ", " << last_set << "])\n";
	}

	// create the block access
	BlockAccess *accs = 0;
	if(blocks) {
		accs = new BlockAccess[blocks.count()];
		for(int i = 0; i < blocks.count(); i++)
			accs[i] = blocks[i];
	}
	DATA_BLOCKS(bb) = pair(blocks.count(), accs);
	blocks.clear();*/
}

} }	// otawa::dcache
