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
void LBlockBuilder::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	
	// Check the cache
	cache = fw->platform()->cache().instCache();
	if(!cache)
		throw ProcessorException(*this, "No cache in this platform.");
	
	// Build the l-block sets
	lbsets = new LBlockSet *[cache->rowCount()];
	LBLOCKS(fw) = lbsets;
	for(int i = 0; i < cache->rowCount(); i++) {
		lbsets[i] = new LBlockSet(i);
		new LBlock(lbsets[i], 0, 0, 0, -1);
	}


	
	
	// Let's go
	CFGProcessor::processWorkSpace(fw);
	

	
	// Add end blocks
	for(int i = 0; i < cache->rowCount(); i++)
		new LBlock(lbsets[i], 0, 0, 0, -1);
}


/**
 */
void LBlockBuilder::processLBlockSet(WorkSpace *fw, CFG *cfg, LBlockSet *lbset, const hard::Cache *cach, int *tableindex) {
	int line = lbset->line();
	int index;
	
	assert(fw);
	assert(cfg);
	assert(lbset);
	cacheBlocks = new HashTable<int, int>();
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
						address_t next_address = (address_t)((mask_t)(address.address() + cach->blockSize()) & ~(cach->blockSize() - 1));
						int cbid;
						int block = cach->block(address);
						int existing_block_id = cacheBlocks->get(block, -1);
						if (existing_block_id != -1) {
						        cbid = existing_block_id;
                        } else {
                        	cbid = lbset->newCacheBlockID();
                        	cacheBlocks->put(block, cbid);
                        }
                        index = tableindex[bb->number()];
                        tableindex[bb->number()] ++;
                        
                        genstruct::AllocatedTable<LBlock*> *lblocks = BB_LBLOCKS(bb);
						lblocks->set(index, new LBlock(lbset, address, bb, next_address - address, cbid)); 											
						find = true;
					}
				}
			}
		}
		delete cacheBlocks;
}


/**
 */	
void LBlockBuilder::processCFG(WorkSpace *fw, CFG *cfg) {
        int *tableindex;
	assert(fw);
	assert(cfg);

	// Initialization
	const hard::CacheConfiguration& conf = fw->platform()->cache();
	if(!conf.instCache())
		throw ProcessorException(*this, "no instruction cache !");
	const hard::Cache *cach = conf.instCache();
	
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		/* Compute the number of lblocks in this basic block */
		if (bb->size() != 0) {
		        
		   /* The BasicBlock spans at least (bbsize-1)/blocksize cache block boundaries (add +1 for the number of l-blocks) */
/*		   cout << "addr: " << bb->address() << "\n"; */
/*		   cout << "taille: " << bb->size() - 1 << "\n"; */
		   int num_lblocks = ((bb->size() - 1) >> cache->blockBits()) + 1;
/*		   cout << "Original num: " << num_lblocks << "\n"; */

		   /* The remainder of the last computation may also span another cache block boundary. */
		   int temp1 = (bb->address().address() + (num_lblocks << cache->blockBits())) & ~(cache->blockSize() - 1);
/*		   cout << "Fin arrondi au cacheblock sup': " << io::hex(temp1) << "\n"; */
		   int temp2 = bb->address().address() + bb->size();
/*		   cout << "Fin du bloc: " << io::hex(temp2) << "\n"; */
		   if (temp1 < temp2) {
                     num_lblocks++;
                   }
     
                   BB_LBLOCKS(bb) = new genstruct::AllocatedTable<LBlock*>(num_lblocks);
                } else {
                   BB_LBLOCKS(bb) = NULL;
                }	        
    }
        
    tableindex = new int[cfg->countBB()];
    for (int i = 0; i < cfg->countBB(); i++)
    	tableindex[i] = 0;
    
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, cfg, lbsets[i], cach, tableindex);
	delete [] tableindex;	
}


/**
 * This feature ensures that the L-blocks of the current task has been
 * collected.
 * 
 * @par Properties
 * @li @ref LBLOCKS
 * @li @ref BB_LBLOCKS
 */
Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE("otawa::lblocks");


} // otawa