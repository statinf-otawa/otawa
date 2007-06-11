
#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ipet/TrivialInstCacheManager.h>
#include <elm/util/Pair.h>

#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/LinkedBlocksDetector.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {
	
  
/**
 * @class LinkedBlocksDetector
 *
 * This processor computes the list of l-blocks which are FIRST_MISS and occupies the same cache block.
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref ICACHE_CATEGORY_FEATURE
 *
 * @par Provided features
 * 
 * @par Statistics
 * none
 */



LinkedBlocksDetector::LinkedBlocksDetector(void) : Processor("otawa::LinkedBlocksDetector", Version(1, 0, 0)), _explicit(false) {
	require(ASSIGNED_VARS_FEATURE);
	require(ICACHE_CATEGORY_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
}

void LinkedBlocksDetector::configure(const PropList& props) {
        Processor::configure(props);
	_explicit = EXPLICIT(props);
}
                
void LinkedBlocksDetector::processWorkSpace(otawa::WorkSpace *fw) {
	const hard::Cache *cache = fw->platform()->cache().instCache();
	ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
	int penalty = cache->missPenalty();
	LBlockSet **lbsets = LBLOCKS(fw);
	
	
	for (int i = 0 ; i < cache->lineCount(); i++) {
		genstruct::Vector<LinkedBlockList*> blockList;
		int count = lbsets[i]->cacheBlockCount();
		for (int j = 0; j < count; j++)
			blockList.add(NULL);
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++) {
			if ((lblock->id() == 0) || (lblock->id() == (lbsets[i]->count() - 1)))
				continue; /* Skip first / last l-blocks */
			if (CATEGORY(lblock) == FIRST_MISS) {
				BasicBlock *header = CATEGORY_HEADER(lblock);
				int cbid = lblock->cacheblock();
				
				if (blockList[cbid] == NULL)
					blockList[cbid] = new LinkedBlockList(order);
				blockList[cbid]->add(lblock);	
			}
		}
		
		for (int j = 0; j < count; j++) {
			if (blockList[j] != NULL) {
						
				Vector<LBlock*> equiv;
				BasicBlock *old_header = NULL;
				equiv.clear();
				for (LinkedBlockList::Iterator iter(*blockList[j]); iter; iter++) {	
					/* We want to build another "equiv" set from scratch whenever the firstmiss-header changes */ 
					BasicBlock *header = CATEGORY_HEADER(*iter);
					if ((old_header) && (old_header != header)) {
						recordBlocks(&equiv);
						equiv.clear();
					}
			
					equiv.add(*iter);
					old_header = header;
				}
				recordBlocks(&equiv);
			}			
		} 
		
		for (int j = 0; j < blockList.length(); j++) {
			if (blockList[j] != NULL)
				delete blockList[j];
		}
	}
}

void LinkedBlocksDetector::recordBlocks(Vector<LBlock*> *equiv) {
	if (equiv->length() == 1)
		return;		
	genstruct::Vector<LBlock*> *copy = new genstruct::Vector<LBlock*>(*equiv);
	for (genstruct::Vector<LBlock*>::Iterator lblock(*equiv); lblock; lblock++) {
		assert(CATEGORY(lblock) == FIRST_MISS);
		LINKED_BLOCKS(lblock) = copy;   		
	}
}

Identifier<genstruct::Vector<LBlock*> *> LINKED_BLOCKS("otawa.cache.linked_blocks", NULL);

}
