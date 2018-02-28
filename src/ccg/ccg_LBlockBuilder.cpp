/*
 *	ccg::LBlockBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT - UPS.
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

#include <otawa/ccg/LBlockBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Memory.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace ccg {

/**
 * @class LBlockBuilder
 * This processor builds the list of l-blocks for each lines of instruction
 * cache and stores it in the CFG.
 *
 * @par Required Features
 * @li @ref INVOLVED_CFGS_FEATURE
 * @li @ref require(hard::CACHE_CONFIGURATION_FEATURE)
 * @li @ref require(hard::MEMORY_FEATURE)
 *
 * @par Provided Features
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 */

p::declare LBlockBuilder::reg = p::init("otawa::ccg::LBlockBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.require(COLLECTED_CFG_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(COLLECTED_LBLOCKS_FEATURE)
	.maker<LBlockBuilder>();


/**
 * Build a new l-block builder.
 */
LBlockBuilder::LBlockBuilder(AbstractRegistration& r): BBProcessor(r), lbsets(0), cache(0), mem(0) {
}


/**
 */
void LBlockBuilder::setup(WorkSpace *fw) {
	ASSERT(fw);

	// check the cache
	cache = 0;
	const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION(fw);
	if(conf)
	cache = conf->instCache();
	if(!cache)
		throw ProcessorException(*this, "No cache in this platform.");

	// get the memory
	mem = hard::MEMORY(fw);

	// Build the l-block sets
	lbsets = new LBlockSet *[cache->rowCount()];
	LBLOCKS(fw) = lbsets;
	for(int i = 0; i < cache->rowCount(); i++) {
		lbsets[i] = new LBlockSet(i, cache);
		new LBlock(lbsets[i], 0, 0, 0, 0);
		ASSERT(lbsets[i]->cacheBlockCount() == 1);
	}
}


/**
 */
void LBlockBuilder::cleanup(WorkSpace *fw) {
	ASSERT(fw);

	// Add end blocks
	for(int i = 0; i < cache->rowCount(); i++)
		new LBlock(lbsets[i], 0, 0, 0, lbsets[i]->cacheBlockCount());
}


/**
 * Add an lblock to the lblock lists.
 * @param bb		Basic block containing the l-block.
 * @param inst		Starting instruction of L-Block to create..
 * @param index		Index in the BB lblock table.
 * @paramlblocks	BB lblock table.
 */
void LBlockBuilder::addLBlock(BasicBlock *bb, Inst *inst, int& index, AllocArray<LBlock*> *lblocks) {

	// test if the l-block is cacheable
	Address addr = inst->address();
	const hard::Bank *bank = mem->get(addr);
	if(!bank)
		log << "WARNING: no memory bank for code at " << addr << ": block considered as cached.\n";
	else if(!bank->isCached()) {
		if(isVerbose())
			log << "INFO: block " << addr << "not cached.\n";
		return;
	}

	// compute the cache block ID
	LBlockSet *lbset = lbsets[cache->set(addr)];
	ot::mask block = cache->block(inst->address());
	int cid = block_map.get(block, -1);
	if(cid < 0) {
		cid = lbset->cacheBlockCount();
		block_map.put(block, cid);
	}

	// Compute the size
	Address top = (addr + cache->blockMask()) & ~cache->blockMask();
	if(top > bb->address() + bb->size())
		top = bb->address() + bb->size();

	// Build the lblock
	LBlock *lblock = new LBlock(lbset, bb, inst, top - addr, cid);
	lblocks->set(index, lblock);
	if(isVerbose())
		log << "\t\t\t\tblock at " << addr
			<< " (cache block " << cache->round(inst->address())
			<< ", cid = " << cid << ")\n";
	index++;
}


/**
 */
void LBlockBuilder::processBB(WorkSpace *fw, CFG *cfg, Block *block) {
	ASSERT(fw);
	ASSERT(cfg);
	ASSERT(block);

	// Do not process entry and exit
	if(!block->isBasic())
		return;
	BasicBlock *bb = block->toBasic();

	// Allocate the BB lblock table
	int num_lblocks =
		((bb->address() + bb->size() + cache->blockMask()).offset() >> cache->blockBits())
		- (bb->address().offset() >> cache->blockBits());
	AllocArray<LBlock*> *lblocks = new AllocArray<LBlock*>(num_lblocks);
	BB_LBLOCKS(bb) = lblocks;

	// Traverse instruction
	int index = 0;
	hard::Cache::set_t set = cache->set(bb->first()->address()) - 1;
	for(BasicBlock::InstIter inst = bb->insts(); inst; inst++)
		if(set != cache->set(inst->address())) {
			set = cache->set(inst->address());
			addLBlock(bb, inst, index, lblocks);
		}
	ASSERT(index == num_lblocks);
}


/**
 * @class LBlock
 * The l-block is an abstraction allowing to split the basic block
 * according to the bound of the cache block, that is, instructions composing
 * a l-block are owned by the same basic block and same cache block. Therefore,
 * when the first one is load in the cache, the next instructions are also load.
 */

/**
 * Build a new LBlock.
 * @param lbset			L-block set which owns this l-block.
 * @param inst			Instruction it starts with.
 * @param bb			Basic block containing this l-block.
 * @param size			Size of the l-block.
 * @param cache_indx	Cache index.
 */
LBlock::LBlock(LBlockSet *lbset, BasicBlock *bb, Inst *inst, t::uint32 size, int cache_index)
: lbs(lbset), _inst(inst), _size(size), _bb(bb), cid(cache_index) {
	idx = lbset->LBlockSet::add(this);
}


/**
 * @fn int LBlock::cacheBlock(void) const;
 * Compute the cache block of this L-block.
 * @return	Cache block.
 */


/**
 */
LBlock::~LBlock(void) {
	delete this;
};


/**
 * @fn size_t LBlock::size(void) const;
 * Get the size of the current l-block.
 * @return	L-block size.
 */


/**
 * Count the instructions in the l-block.
 * @return	L-block instruction count.
 */
int LBlock::countInsts(void) {
	int cnt = 0;
	Address addr = _inst->address();

	if(_bb != 0)
		for(BasicBlock::InstIter instr(_bb); instr; instr++)
			if(instr->address() >= addr && instr->address() < addr + _size)
				cnt++;

	return cnt;
}


/**
 * @fn int LBlock::number(void) const;
 * Get the number of the LBlock in its cache row.
 * This numbers span from 0 to LBlockSet::count()  - 1.
 * @return	Number of the l-block.
 */


/**
 * @fn int LBlock::id(void) const;
 * @deprecated	Replaced by number().
 */


/**
 * @fn bool LBlock::sameCacheBlock(const LBlock *block) const;
 * Test if both l-blocks belong to the same cache block.
 * @param block		Block to test with.
 * @return			True if they belongs to the same cache block, false else.
 */


/**
 * @fn int LBlock::cacheblock(void);
 * Get a unique number identifying which cache block the l-block belongs to.
 * @param	Cache block number.
 * @deprecated	Currently, there is no way to get back relation with the cache address.
 * For testing if two l-blocks matches the same cache block, uses sameCacheBlock() instead.
 */


/**
 * @fn address_t LBlock::address(void)
 * Get address of the l-block.
 * @return	L-block address.
 */


/**
 * @fn BasicBlock *LBlock::bb(void);
 * Get the BB containing the l-block.
 * @return	Container BB.
 */


/**
 */
Output& operator<<(Output& out, const LBlock *block) {
	out << "lblock(" << block->lblockset()->line() << ", " << block->number() << ")";
	return out;
}



/**
 * This property is used for storing the list of L-Blocks. The type of its
 * date is LBlockSet *[] with a size equal to the line count of the instruction
 * cache.
 *
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<LBlockSet **> LBLOCKS("otawa::ccg::LBLOCKS", 0);

/**
 * This property is used for storing the list of L-Blocks of a BasicBlock.
 * cache.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<AllocArray<LBlock* >* > BB_LBLOCKS("otawa::ccg::BB_LBLOCKS", 0);


/**
 * @class LBlockSet
 * This class represents the list of l-blocks of a task for a chosen cache row.
 * It is useful to perform analysis on l-blocks. It is accessible through
 * the property @ref otawa::LBLOCKS.
 */


/**
 * Add a new l-block.
 * @param lblock	l-block to add.
 */
int LBlockSet::add(LBlock *lblock){
	int id = listelbc.length();
	listelbc.add(lblock);
	if(lblock->cacheBlock() >= cblock_count)
		cblock_count = lblock->cacheBlock() + 1;
	return id;
}


/**
 * Build a l-block set.
 * @param line	Cache row of the l-block set.
 */
LBlockSet::LBlockSet(int line, const hard::Cache *cache): linenumber(line), cblock_count(0), _cache(cache) {
	ASSERT(line >= 0);
}


/**
 * Get a number for a new l-block in the set.
 * Used internally to build l-blocks.
 * @return	New l-block number.
 */
/*int LBlockSet::newCacheBlockID(void) {
	_cacheBlockCount++;
	return(_cacheBlockCount-1);
}*/


/**
 * @fn int LBlockSet::count(void);
 * Get the count of l-blocks in the set.
 * @return	Count of l-blocks.
 */


/**
 * @fn int LBlockSet::cacheBlockCount(void);
 * Get the internal counter of l-block number
 * (only used internally).
 */


/**
 * @fn LBlock *LBlockSet::lblock(int i);
 * Get the l-block with the given index.
 * @param i		Index of the looked block.
 * @return		L-block matching the index.
 */


/**
 * @fn int LBlockSet::set(void);
 * Get the set number of the l-blocks in the cache.
 * @return	Cache set number.
 */

/**
 * This feature ensures that the L-blocks of the current task has been
 * collected.
 *
 * @par Properties
 * @li @ref LBLOCKS
 * @li @ref BB_LBLOCKS
 */
p::feature COLLECTED_LBLOCKS_FEATURE("otawa::ccg::COLLECTED_LBLOCKS_FEATURE", new Maker<LBlockBuilder>());

} }	// otawa
