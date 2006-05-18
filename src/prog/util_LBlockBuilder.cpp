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
#include <otawa/ipet/IPET.h>

namespace otawa {

/**
 * @class LBlockBuilder
 * This processor builds the list of l-blocks for each lines of instruction
 * cache and stores it in the CFG.
 */


/**
 */
void LBlockBuilder::processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset) {
	assert(fw);
	assert(cfg);
	assert(lbset);

	// Initialization
	ilp::System *system = cfg->get<ilp::System *>(ipet::IPET::ID_System, 0);
	assert (system);
	const hard::Cache *cach = fw->platform()->cache().instCache();
	
	// Create entry node
	new LBlock(lbset, 0, 0, 0);
	
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
					if(pseudo->id() == bb->ID)
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
	
	// Create exit node
	new LBlock(lbset, 0, 0, 0);
}


/**
 * Build a new l-block builder.
 * @param props		Configuration properties.
 */
// Inlines
LBlockBuilder::LBlockBuilder(const PropList& props)
: CFGProcessor("otawa::util::LBlockBuilder", Version(1, 0, 0), props) {
}


/**
 */	
void LBlockBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(fw);
	assert(cfg);
	
	// Check the cache
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		throw new ProcessorException(*this, "No cache in this platform.");
	
	// Build the l-block sets
	LBlockSet **lbsets = new LBlockSet *[cache->lineCount()];
	cfg->set(LBlockSet::ID_LBlockSet, lbsets);
	
	// Initialize the l-block sets
	for(int i = 0; i < cache->lineCount(); i++) {
		lbsets[i] = new LBlockSet(i);
		processLBlockSet(fw, cfg, lbsets[i]); 
	}
}

} // otawa
