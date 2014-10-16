/*
 *	LBlockBuilder implementation
 *	Copyright (c) 2005-12, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class implementation
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/assert.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Memory.h>
#include <otawa/hard/Platform.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/IPET.h>
#include <otawa/cfg/CFGCollector.h>
#include <elm/genstruct/Vector.h> 
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/Table.h>

namespace otawa {

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

p::declare LBlockBuilder::reg = p::init("otawa::util::LBlockBuilder", Version(1, 1, 0))
	.base(BBProcessor::reg)
	.require(COLLECTED_CFG_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(COLLECTED_LBLOCKS_FEATURE)
	.maker<LBlockBuilder>();


/**
 * Build a new l-block builder.
 */
LBlockBuilder::LBlockBuilder(AbstractRegistration& r): BBProcessor(r), lbsets(0), cache(0) {
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
void LBlockBuilder::addLBlock(BasicBlock *bb, Inst *inst, int& index, genstruct::AllocatedTable<LBlock*> *lblocks) {
	
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
		int set = cache->set(inst->address());
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
void LBlockBuilder::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
	ASSERT(fw);
	ASSERT(cfg);
	ASSERT(bb);
	
	// Do not process entry and exit
	if (bb->isEnd())
		return;
		
	// Allocate the BB lblock table
	int num_lblocks =
		((bb->address() + bb->size() + cache->blockMask()) >> cache->blockBits())
		- (bb->address() >> cache->blockBits());
	genstruct::AllocatedTable<LBlock*> *lblocks = new genstruct::AllocatedTable<LBlock*>(num_lblocks);
	BB_LBLOCKS(bb) = lblocks;

	// Traverse instruction
	int index = 0;
	hard::Cache::set_t set = cache->set(bb->firstInst()->address()) - 1;
	for(BasicBlock::InstIterator inst(bb); inst; inst++)
		if(set != cache->set(inst->address())) {
			set = cache->set(inst->address());
			addLBlock(bb, inst, index, lblocks);
		}
	ASSERT(index == num_lblocks);
}


/**
 * This properties is set on the first instruction of each lblock and points to the lblock
 *
 * @par Hooks
 * @li @ref Inst
 */
//Identifier<LBlock *> LBLOCK("otawa::LBLOCK");

/**
 * This feature ensures that the L-blocks of the current task has been
 * collected.
 * 
 * @par Properties
 * @li @ref LBLOCKS
 * @li @ref BB_LBLOCKS
 */
Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE("otawa::COLLECTED_LBLOCKS_FEATURE");


} // otawa
