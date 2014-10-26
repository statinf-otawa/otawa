/*
 *	dcache::BlockBuilder class implementation
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

#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/Memory.h>
#include <otawa/hard/Platform.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/Process.h>
#include <otawa/dcache/BlockBuilder.h>
#include <otawa/stack/AccessedAddress.h>

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
 * @ingroup dcache
 */
p::declare BlockBuilder::reg = p::init("otawa::dcache::BlockBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<BlockBuilder>()
	.provide(DATA_BLOCK_FEATURE)
	.require(ADDRESS_ANALYSIS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);


/**
 * Provide the address of the stack pointer at the start of the task.
 */
Identifier<Address> INITIAL_SP("otawa::dcache::INITIAL_SP", Address::null);


/**
 */
BlockBuilder::BlockBuilder(p::declare& r): BBProcessor(r), mem(0), cache(0), colls(0), man(0) {
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
	mem = hard::MEMORY(ws);
	if(!mem) {
		if(isVerbose())
			log << "INFO: no workspace memory. Using default memory.\n";
		mem = &ws->platform()->memory();
	}
	if(!sp)
		sp = ws->process()->defaultStack();
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
			break;
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
		if(!bank) {
			const genstruct::Table<const hard::Bank *>& banks = mem->banks();
			for(int i = 0; i < banks.count(); i++)
				log << "INFO: " << banks[i]->address() << " " << banks[i]->size() << io::endl;
			throw otawa::Exception(_ << "no memory bank for address " << addr
					<< " accessed from " << aa->instruction()->address());
		}
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
 * @ingroup dcache
 */
SilentFeature DATA_BLOCK_FEATURE("otawa::dcache::DATA_BLOCK_FEATURE", maker);


/**
 * Gives the list of used blocks in the data cache. Its argument is an array of block collections,
 * one for each cache set. Therefore, the array size is equal to the number of cache sets.
 *
 * @p Hooks
 * @li @ref	WorkSpace
 * @ingroup dcache
 */
Identifier<const BlockCollection *> DATA_BLOCK_COLLECTION("otawa::dcache::DATA_BLOCK_COLLECTION", 0);


/**
 * Give the list of accesses to the data cache. The first member of the pair
 * represents the number of block accesses in the array whose pointer is in the second member.
 *
 * @p Hooks
 * @li @ref BasicBlock
 * @ingroup dcache
 */
Identifier<Pair<int, BlockAccess *> > DATA_BLOCKS("otawa::dcache::DATA_BLOCKS", pair(0, (BlockAccess *)0));


/**
 * @class Block
 * Represents a single block used by the data cache.
 * @ingroup dcache
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
		out << addr << " (" << idx << " in " << _set << ")";
}


/**
 * @class BlockCollection
 * A block collections stores the list of data blocks used in a task
 * for a specific line.
 * @ingroup dcache
 */


/**
 */
BlockCollection::~BlockCollection(void) {
	for(int i = 0; i < blocks.count(); i++)
		delete blocks[i];
}

/**
 * Obtain a block matching the given address.
 * @param addr	Address of the looked block.
 * @return		Matching block (possibly created).
 */
const Block& BlockCollection::obtain(const Address& addr) {
	ASSERT(!addr.isNull());
	for(int i = 0; i < blocks.count(); i++)
		if(addr == blocks[i]->address())
			return *blocks[i];
	blocks.add(new Block(_set, blocks.count(), addr));
	return *blocks.top();
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
 * A block access represents a data memory access of an instruction.
 *
 * The action is defined by BlockAccess::action_t that may be:
 * @li @ref NONE -- invalid action (only for convenience),
 * @li @ref READ -- read of cache,
 * @li @ref WRITE -- write of cache,
 * @li @ref PURGE -- target block are purged (possibly written back to memory).
 *
 * Possible kinds of data accesses include:
 * @li ANY		Most imprecised access: one memory accessed is performed but the address is unknown.
 * @li BLOCK	A single block is accessed (given by @ref block() method).
 * @li RANGE	A range of block may be accessed (between @ref first() and @ref last() methods addresses).
 * @ingroup dcache
 */

/**
 * @fn BlockAccess::BlockAccess(void);
 * Build a null block access.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction, action_t action);
 * Build a block access of type ANY.
 * @param instruction	Instruction performing the access.
 * @param action		Type of action.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction, const Block& block);
 * Build a block access to a single block.
 * @param instruction	Instruction performing the access.
 * @param action		Type of action.
 * @param block			Accessed block.
 */

/**
 * @fn BlockAccess::BlockAccess(Inst *instruction, action_t action, Address::offset_t first, Address::offset_t last);
 * Build a block access of type range. Notice the address of first block may be
 * greater than the address of the second block, meaning that the accessed addresses
 * ranges across the address modulo by 0.
 * @param instruction	Instruction performing the access.
 * @param action		Type of action.
 * @param first			First accessed block (must a cache block boundary address).
 * @param last			Last access block (must a cache block boundary address).
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
 * @fn action_t BlockAccess::action(void) const;
 * Get the performed action.
 * @return	Performed action.
 */

/**
 * @fn const Block& BlockAccess::block(void) const;
 * Only for kind BLOCK, get the accessed block.
 * @return	Accessed block.
 */

/**
 * @fn Address::offset_t BlockAccess::first(void) const;
 * Only for the RANGE kind, get the first accessed block.
 * @return	First accessed block.
 */

/**
 * @fn Address::offset_t BlockAccess::last(void) const
 * Only for the RANGE kind, get the last accessed block.
 * @return	Last accessed block.
 */


/**
 * @fn bool BlockAccess::inRange(Address::offset_t block) const;
 * Test if the given block is the range of the given access.
 * @param block		Address of the cache block.
 * @return			True if it is in the range, false else.
 */

/**
 */
void BlockAccess::print(io::Output& out) const {
	ASSERTP(_action <= PURGE, "invalid block access action: " << _action);
	out << inst->address() << " (" << inst << "): " << action_t(_action) << ' ';
	switch(_kind) {
	case ANY: 	out << "ANY"; break;
	case BLOCK: out << *data.blk; break;
	case RANGE: out << '[' << data.range.first << ", " << data.range.last << ']'; break;
	default:	ASSERTP(false, "invalid block access kind: " << _kind); break;
	}
}

io::Output& operator<<(io::Output& out, BlockAccess::action_t action) {
	static cstring action_names[] = {
		"none",		// NONE = 0
		"load",		// READ = 1
		"store",	// WRITE = 2
		"purge"		// PURGE = 3
	};
	out << action_names[action];
	return out;
}

} }	// otawa::dcache
