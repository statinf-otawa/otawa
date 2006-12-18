/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/util_LBlockBuilder.cpp -- interface of LBlockBuilder class.
 */

#include <assert.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/IPET.h>
#include <otawa/cfg/CFGCollector.h>

namespace otawa {

/**
 * @class LBlockBuilder
 * This processor builds the list of l-blocks for each lines of instruction
 * cache and stores it in the CFG.
 * 
 * @par Required Features
 * @li @ref INVOLVED_CFGS_FEATURE
 * 
 * @par Provided Features
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 */


/**
 * Build a new l-block builder.
 */
LBlockBuilder::LBlockBuilder(void)
: CFGProcessor("otawa::util::LBlockBuilder", Version(1, 0, 0)) {
	require(COLLECTED_CFG_FEATURE);
	provide(COLLECTED_LBLOCKS_FEATURE);
}


/**
 */
void LBlockBuilder::processFrameWork(FrameWork *fw) {
	assert(fw);
	
	// Check the cache
	cache = fw->platform()->cache().instCache();
	if(!cache)
		throw new ProcessorException(*this, "No cache in this platform.");
	
	// Build the l-block sets
	lbsets = new LBlockSet *[cache->lineCount()];
	LBLOCKS(fw) = lbsets;
	for(int i = 0; i < cache->lineCount(); i++) {
		lbsets[i] = new LBlockSet(i);
		new LBlock(lbsets[i], 0, 0, 0);
	}

	// Let's go
	CFGProcessor::processFrameWork(fw);
	
	// Add end blocks
	for(int i = 0; i < cache->lineCount(); i++)
		new LBlock(lbsets[i], 0, 0, 0);
}


/**
 */
void LBlockBuilder::processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset) {
	assert(fw);
	assert(cfg);
	assert(lbset);

	// Initialization
	const hard::CacheConfiguration& conf = fw->platform()->cache();
	if(!conf.instCache())
		throw ProcessorException(*this, "no instruction cache !");
	const hard::Cache *cach = conf.instCache();
	
	// Build the l-blocks
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
	
		if (!bb->isEntry() && !bb->isExit()) {
			
			// ilp::Var *bbv = bb->use<ilp::Var *>(ipet::IPET::ID_Var);
			Inst *inst;
			bool find = false;
			PseudoInst *pseudo;
			
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {				
				pseudo = inst->toPseudo();
				address_t address = inst->address();

				// Do not process pseudo or stop on BB start pseudo
				if(pseudo) {
					if(pseudo->id() == &bb->ID)
						break;
				}
				
				// Process the instruction
				else {
					
					if(cach->line(address) != lbset->line())
						find = false;
						
					if(!find && cach->line(address) == lbset->line()) {
						address_t next_address = (address_t)((mask_t)(address + cach->blockSize()) & ~(cach->blockSize() - 1));
						new LBlock(lbset, address, bb, next_address - address);
						find = true;
					}
				}
			}
		}
}


/**
 */	
void LBlockBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(fw);
	assert(cfg);
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, cfg, lbsets[i]); 
}


/**
 * This feature ensures that the L-blocks of the current task has been
 * collected.
 * 
 * @par Properties
 * @li @ref LBLOCKS
 */
Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE("otawa::lblocks");

} // otawa
