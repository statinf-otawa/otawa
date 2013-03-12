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

namespace otawa { namespace dcache {

/**
 * @class BlockBuilder
 * Build the list of blocks used for L1 data cache analysis and decorate each basic block
 * with the list of performed accesses.
 *
 * @p Provided Features
 * @li @ref DATA_BLOCK_FEATURE
 *
 * @p Required Features
 * @li @ref ADDRESS_ANALYSIS_FEATURE
 *
 * @p Configuration
 * @li @ref INITIAL_SP
 */
p::declare BlockBuilder::reg = p::init("otawa::dcache::BlockBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<BlockBuilder>()
	.provide(DATA_BLOCK_FEATURE)
	.require(ADDRESS_ANALYSIS_FEATURE);


/**
 * Provide the address of the stack pointer at the start of the task.
 */
Identifier<Address> INITIAL_SP("otawa::dcache::INITIAL_STACK", Address::null);


/**
 */
BlockBuilder::BlockBuilder(p::declare& r): BBProcessor(r), mem(0), cache(0), colls(0) {
}


/**
 */
void BlockBuilder::configure(const PropList &props) {
	BBProcessor::configure(props);
	sp = INITIAL_SP(props);
}


/**
 */
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
	if(logFor(LOG_PROC))
		log << "\tinitial SP = " << sp << io::endl;
	colls = new BlockCollection[cache->rowCount()];
	DATA_BLOCK_COLLECTION(ws) = colls;
	// !!TODO!! add cleanup code when ported to recent OTAWA
	for(int i = 0; i < cache->rowCount(); i++)
		colls[i].setSet(i);
}


/**
 */
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
			const Block& block = colls[set].get(set, addr);
			blocks.add(BlockAccess(aa->instruction(), block));
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access " << addr
					<< " (" << block.index() << ", " << block.set() << ")\n";
			continue;
		}

		// range over the full cache ?
		last = Address(last.page(), (last.offset() - 1) & ~cache->blockMask());
		if(last - addr >= cache->cacheSize()) {
			blocks.add(BlockAccess(aa->instruction()));
			if(logFor(LOG_INST))
				log << "\t\t\t\t" << aa->instruction() << " access any [" << addr << ", " << last << ")\n";
			continue;
		}

		// a normal range
		int last_set = cache->line(last.offset());
		blocks.add(BlockAccess(aa->instruction(), set, last_set));
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
	blocks.clear();
}

static SilentFeature::Maker<BlockBuilder> maker;
/**
 * This feature ensures that information about the data cache accesses has been provided
 * on each basic block.
 *
 * @p Default processor
 * @li @ref BlockBuilder
 *
 * @p Properties
 * @li @ref DATA_BLOCK_COLLECTION
 * @li @ref DATA_BLOCKS
 */
SilentFeature DATA_BLOCK_FEATURE("otawa::dcache::DATA_BLOCK_FEATURE", maker);


/**
 * Gives the list of used blocks in the data cache. Its argument is an array of block collections,
 * one for each cache set. Therefore, the array size is equal to the number of cache sets.
 *
 * @p Hooks
 * @li @ref	WorkSpace
 */
Identifier<const BlockCollection *> DATA_BLOCK_COLLECTION("otawa::dcache::DATA_BLOCK_COLLECTION", 0);


/**
 * Give the list of accesses to the data cache. The first member of the pair
 * represents the number of block accesses in the array whose pointer is in the second member.
 *
 * @p Hooks
 * @li @ref BasicBlock
 */
Identifier<Pair<int, BlockAccess *> > DATA_BLOCKS("otawa::dcache::DATA_BLOCKS", pair(0, (BlockAccess *)0));


/**
 * @class Block
 * Represents a single block used by the data cache.
 */

/**
 * @fn Block::Block(void);
 * Build a "any" number (with a null address).
 */

/**
 * @fn Block::Block(int set, int index, const Address& address);
 * Build a simple block.
 * @param set		Number of its cache set.
 * @param index		Its number.
 * @param address	Address of the block.
 */

/**
 * @fn Block::Block(const Block& block);
 * Cloning of a block.
 * @param block		Block to clone.
 */

/**
 * @fn int Block::set(void) const;
 * Get the set number of a block.
 * @return		Set number.
 */

/**
 * @fn int Block::index(void) const;
 * Get the number of a block.
 * @return	Block number.
 */


/**
 * @fn const Address& address(void) const;
 * Get the address of a block.
 * @eturn	Block address.
 */


/**
 * Print a block.
 */
void Block::print(io::Output& out) const {
	if(_set == -1)
		out << "ANY";
	else
		out << addr << " (" << idx << ", " << _set << ")";
}


/**
 * @class BlockCollection
 * A block collections stores the list of data blocks used in a task
 * for a specific line.
 */


/**
 * Obtain a block matching the given address.
 * @param set	Set of the block.
 * @param addr	Address of the looked block.
 * @return		Matching block (possibly created).
 */
const Block& BlockCollection::get(int set, const Address& addr) {
	for(int i = 0; i < blocks.count(); i++)
		if(addr == blocks[i].address())
			return blocks[i];
	blocks.add(Block(set, blocks.count(), addr));
	return blocks[blocks.count() - 1];
}

/**
 * @fn void BlockCollection::setSet(int set);
 * Set the set number of the block collection.
 * @param set	Set number.
 */

/**
 * @fn  int BlockCollection::count(void);
 * Get the count of blocks in this collection.
 * @return	Count of blocks.
 */

/**
 * @fn int BlockCollection::set(void) const;
 * Get the cache set of the current collection.
 * @return	Cache set.
 */


/**
 * @class BlockAccess
 * A block access represents a data memory of an instruction.
 * Possible kinds of data accesses include:
 * @li ANY		Most imprecised access: one memory accessed is performed but the address is unknown.
 * @li BLOCK	A single block is accessed (given by @ref block() method).
 * @li RANGE	A range of block may be accessed (between @ref first() and @ref last() methods addresses).
 */

/**
 * @fn BlockAccess::BlockAccess(void);
 * Build a null block access.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction);
 * Build a block access of type ANY.
 * @param instruction	Instruction performing the access.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction, const Block& block);
 * Build a block access to a single block.
 * @param instruction	Instruction performing the access.
 * @param block			Accessed block.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction, int first, int last);
 * Build a block access of type range. Notice the address of first block may be
 * greater than the address of the second block, meaning that the accessed addresses
 * ranges across the address modulo by 0.
 * @param instruction	Instruction performing the access.
 * @param first			First accessed block.
 * @param last			Last access block.
 */

/**
 * @fn BlockAccess::BlockAccess(const BlockAccess& acc);
 * Construction by cloning.
 * @param acc	Cloned access.
 */

/**
 * @fn BlockAccess::BlockAccess& operator=(const BlockAccess& acc);
 * Block access assignement.
 * @param acc	Assigned access.
 * @return		Current block.
 */

/**
 * @fn Inst *BlockAccess::instruction(void) const;
 * Get the instruction performing the access.
 * @return	Instruction performing the access (must be an instruction of the basic block the access is applied to).
 */

/**
 * @fn kind_t BlockAccess::kind(void);
 * Get the kind of the access.
 * @return	Access kind.
 */

/**
 * @fn const Block& BlockAccess::block(void) const;
 * Only for kind BLOCK, get the accessed block.
 * @return	Accessed block.
 */

/**
 * @fn int BlockAccess::first(void) const;
 * Only for the RANGE kind, get the first accessed block.
 * @return	First accessed block.
 */

/**
 * @fn int BlockAccess::last(void) const
 * Only for the RANGE kind, get the last accessed block.
 * @return	Last accessed block.
 */

/**
 */
void BlockAccess::print(io::Output& out) const {
	out << inst << " access ";
	switch(_kind) {
	case ANY: out << "ANY"; break;
	case BLOCK: out << *data.blk; break;
	case RANGE: out << '[' << data.range.first << ", " << data.range.last << ']'; break;
	}
}

} }	// otawa::dcache
