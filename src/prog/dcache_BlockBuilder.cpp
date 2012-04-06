/*
 * DataBlockBuilder.cpp
 *
 *  Created on: 9 juil. 2009
 *      Author: casse
 */

#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/Memory.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Process.h>
#include <otawa/dcache/BlockBuilder.h>
#include <otawa/util/AccessedAddress.h>

namespace otawa {

Identifier<Pair<int, BlockAccess *> > DATA_BLOCKS("otawa::DATA_BLOCKS", pair(0, (BlockAccess *)0));
Identifier<Address> INITIAL_SP("otawa::INITIAL_STACK", Address::null);


BlockBuilder::BlockBuilder(void): BBProcessor("otawa::BlockBuilder") {
	provide(DATA_BLOCK_FEATURE);
	require(ADDRESS_ANALYSIS_FEATURE);
}


void BlockBuilder::configure(const PropList &props) {
	BBProcessor::configure(props);
	sp = INITIAL_SP(props);
}

void BlockBuilder::setup(WorkSpace *ws) {
	cache = hard::CACHE_CONFIGURATION(ws)->dataCache();
	if(!cache)
		throw otawa::Exception("no data cache !");
	if(cache->replacementPolicy() != hard::Cache::LRU)
		throw otawa::Exception("unsupported replacement policy in data cache !");
	mem = &ws->process()->platform()->memory();
	if(!sp)
		sp = ws->process()->initialSP();
	if(!sp)
		throw otawa::Exception("no valid SP address");
	if(isVerbose())
		log << "\tinitial SP = " << sp << io::endl;
	colls = new BlockCollection[cache->rowCount()];
	DATA_BLOCK_COLLECTION(ws) = colls;
	// !!TODO!! add cleanup code when ported to recent OTAWA
	for(int i = 0; i < cache->rowCount(); i++)
		colls[i].setSet(i);
}


void BlockBuilder::processBB (WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	AccessedAddresses *addrs = ADDRESSES(bb);
	if(!addrs)
		return;

	// compute block accessed
	for(int i = 0; i < addrs->size(); i++) {
		Address last;
		AccessedAddress *aa = addrs->get(i);

		// get address
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

		// access any ?
		if(addr.isNull()) {
			blocks.add(BlockAccess(aa->instruction()));
			if(isVerbose())
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
			if(isVerbose())
				log << "\t\t\t\t" << aa->instruction() << " access not cached "
					<< addr << "\n";
			continue;
		}

		// create the block access if any
		addr = Address(addr.page(), addr.offset() & ~cache->blockMask());
		int set = cache->line(addr.offset());
		if(last.isNull()) {
			const Block& block = colls[set].get(set, addr);
			blocks.add(BlockAccess(aa->instruction(), block));
			if(isVerbose())
				log << "\t\t\t\t" << aa->instruction() << " access " << addr
					<< " (" << block.index() << ", " << block.set() << ")\n";
			continue;
		}

		// range over the full cache ?
		last = Address(last.page(), (last.offset() - 1) & ~cache->blockMask());
		if(last - addr >= cache->cacheSize()) {
			blocks.add(BlockAccess(aa->instruction()));
			if(isVerbose())
				log << "\t\t\t\t" << aa->instruction() << " access any [" << addr << ", " << last << ")\n";
			continue;
		}

		// a normal range
		int last_set = cache->line(last.offset());
		blocks.add(BlockAccess(aa->instruction(), set, last_set));
		if(isVerbose())
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
	blocks.clear();
}

Feature<BlockBuilder> DATA_BLOCK_FEATURE("otawa::DATA_BLOCK_FEATURE");
Identifier<const BlockCollection *> DATA_BLOCK_COLLECTION("otawa::DATA_BLOCK_COLLECTION", 0);

}	// otawa
