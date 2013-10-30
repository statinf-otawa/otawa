/*
 *	$Id$
 *	LinkedBlocksDetector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#include <elm/genstruct/SortedSLList.h>

#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/LinkedBlocksDetector.h>




using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {

using namespace cache;

// Ordered list
class NumberOrder {
public:
	static inline int compare(LBlock *lb1, LBlock *lb2) {
		return cache::CATEGORY_HEADER(lb1)->number() - cache::CATEGORY_HEADER(lb2)->number();
	}
};
typedef genstruct::SortedSLList<LBlock*, NumberOrder> LinkedBlockList;


/**
 * @class LinkedBlocksDetector
 * @author C. Ballabriga <ballabri@irit.fr>
 *
 * This processor computes the list of l-blocks which are FIRST_MISS, and occupies the same cache block,
 * and whose FIRST_MISS loop header is the same.
 * i.e. for each loop, it is computed a vector of l-blocks which are FIRST_MISS for this loop.
 *
 * This is useful for the CAT2ConstraintBuilder, where constraints of type x1miss + x2miss <= 1 can be
 * constructed (instead of x1miss <=1 and x2miss <=1). Indeed, the two blocks together will cause only
 * one miss (since if we access the first l-block, we load the whole cache block, and so the
 * second l-block will cause a hit)
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref ICACHE_CATEGORY_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 *
 * @par Provided features
 *
 * @par Statistics
 * none
 */


/**
 */
LinkedBlocksDetector::LinkedBlocksDetector(void) : Processor("otawa::LinkedBlocksDetector", Version(1, 0, 0)), _explicit(false) {
	require(ASSIGNED_VARS_FEATURE);
	require(ICACHE_CATEGORY2_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
}


/**
 */
void LinkedBlocksDetector::configure(const PropList& props) {
        Processor::configure(props);
	_explicit = EXPLICIT(props);
	cstats = cache::CATEGORY_STATS(props);
}


/**
 */
void LinkedBlocksDetector::processWorkSpace(otawa::WorkSpace *fw) {
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	LBlockSet **lbsets = LBLOCKS(fw);


	for (int i = 0 ; i < cache->rowCount(); i++) {
		genstruct::Vector<LinkedBlockList*> blockList;
		int count = lbsets[i]->cacheBlockCount();
		for (int j = 0; j < count; j++)
			blockList.add(NULL);
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++) {
			if ((lblock->id() == 0) || (lblock->id() == (lbsets[i]->count() - 1)))
				continue; /* Skip first / last l-blocks */
			if (cache::CATEGORY(lblock) == cache::FIRST_MISS) {
				//BasicBlock *header = CATEGORY_HEADER(lblock);
				int cbid = lblock->cacheblock();

				if (blockList[cbid] == NULL)
					blockList[cbid] = new LinkedBlockList();
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
					BasicBlock *header = cache::CATEGORY_HEADER(*iter);
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


/**
 * !!TODO!!
 */
void LinkedBlocksDetector::recordBlocks(Vector<LBlock*> *equiv) {
	if (equiv->length() == 1)
		return;
	genstruct::Vector<LBlock*> *copy = new genstruct::Vector<LBlock*>(*equiv);
	for (genstruct::Vector<LBlock*>::Iterator lblock(*equiv); lblock; lblock++) {
		ASSERT(cache::CATEGORY(lblock) == cache::FIRST_MISS);
		LINKED_BLOCKS(lblock) = copy;
	}
	if(cstats)
		cstats->addLinked();
}


/**
 * !!TODO!!
 */
Identifier<genstruct::Vector<LBlock*> *> LINKED_BLOCKS("otawa::LINKED_BLOCKS", NULL);

}
