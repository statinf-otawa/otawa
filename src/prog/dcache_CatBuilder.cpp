/*
 * DataCatBuilder.cpp
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dcache/CatBuilder.h>
#include <otawa/dcache/ACSMayBuilder.h>

namespace otawa { namespace dcache {

Identifier<BasicBlock*> CATEGORY_HEADER("otawa::dcache::CATEGORY_HEADER", 0);
Identifier<category_t> CATEGORY("otawa::dcache::category", INVALID_CATEGORY);


/**
 */
CATBuilder::CATBuilder(void): Processor("otawa::dcache::CATBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(DCACHE_ACS_FEATURE);
	provide(CACHE_CATEGORY_FEATURE);
}


void CATBuilder::cleanup(WorkSpace *ws) {
	static cstring cat_names[] = {
			"INV",
			"AH",
			"FH",
			"FM",
			"AM",
			"NC"
	};

	if(!isVerbose())
		return;

	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++)
		for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++) {
			cerr << "\tBB " << bb->number() << " (" << bb->address() << ")\n";
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];
				cerr << "\t\t" << b << " -> " << cat_names[dcache::CATEGORY(b)] << io::endl;
			}
		}
}


/**
 * !!TODO!!
 */
void CATBuilder::processLBlockSet(WorkSpace *ws, const otawa::BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;

	// prepare problem
	int line = coll.set();
	MUSTProblem::Domain dom(coll.count(), cache->wayCount());

	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++)
		for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++) {

			// get the input domain
			genstruct::Vector<MUSTProblem::Domain *> *ins = DCACHE_ACS_MUST(bb);
			dom.set(*ins->get(line));

			// explore the adresses
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];
				if(b.kind() != BlockAccess::BLOCK) {
					CATEGORY(b) = NOT_CLASSIFIED;
					dom.ageAll();
				}
				else if(b.block().set() == line) {

					// initialization
					MAYProblem::Domain *may = NULL;
					if(ACS_MAY(bb) != NULL) {
						may = ACS_MAY(bb)->get(line);
						CATEGORY(b) = NOT_CLASSIFIED;
					}
					else
						CATEGORY(b) = ALWAYS_MISS;

					// in MUST
					if(dom.contains(b.block().index()))
						CATEGORY(b) = ALWAYS_HIT;
					else if(may && !may->contains(b.block().index()))
						CATEGORY(b) = ALWAYS_MISS;

					// update state
					dom.inject(b.block().index());
					/*else if (firstmiss_level != FML_NONE) {
						if (LOOP_HEADER(lblock->bb()))
							header = lblock->bb();
					  	else header = ENCLOSING_LOOP_HEADER(lblock->bb());

					  	int bound;
					  	bool perfect_firstmiss = true;
						PERSProblem::Domain *pers = CACHE_ACS_PERS(lblock->bb())->get(line);
						bound = 0;

						if ((pers->length() > 1) && (firstmiss_level == FML_INNER))
							bound = pers->length() - 1;
						CATEGORY_HEADER(lblock) = NULL;
					  	for (int k = pers->length() - 1 ; (k >= bound) && (header != NULL); k--) {
							if (pers->isPersistent(lblock->cacheblock(), k)) {
								CATEGORY(lblock) = FIRST_MISS;
								CATEGORY_HEADER(lblock) = header;
							} else perfect_firstmiss = false;
							header = ENCLOSING_LOOP_HEADER(header);
						}

						if ((firstmiss_level == FML_OUTER) && (perfect_firstmiss == false))
							CATEGORY(lblock) = ALWAYS_MISS;
					}*/
				}
			}
		}

	// Use the results to set the categorization
	/*for (LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		if ((lblock->id() == 0) || (lblock->id() == lbset->count() - 1))
			continue;

		if (LBLOCK_ISFIRST(lblock)) {
		} else {
			CATEGORY(lblock) = ALWAYS_MISS;
		}

		// record stats
		if(cstats)
			cstats->add(CATEGORY(lblock));
	}

*/
}


/**
 */
void CATBuilder::configure(const PropList &props) {
	Processor::configure(props);
	firstmiss_level = DATA_FIRSTMISS_LEVEL(props);
	//cstats = CATEGORY_STATS(props);
	//if(cstats)
	//	cstats->reset();
}


/**
 */
void CATBuilder::processWorkSpace(otawa::WorkSpace *fw) {
	//int i;
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].set());
		processLBlockSet(fw, colls[i], cache );
	}
}


Feature<CATBuilder> CACHE_CATEGORY_FEATURE("otawa::dcache::CACHE_CATEGORY_FEATURE");


/**
 * @class CategoryStats
 * This class is used to store statistics about the categories about cache
 * accesses. It it provided by cache category builders.
 * @see CATBuilder, CAT2Builder
 */

/**
 */
/*CategoryStats::CategoryStats(void) {
	reset();
}*/

/**
 * Reset the statistics.
 */
/*void CategoryStats::reset(void) {
	_total = 0;
	_linked = 0;
	for(int i = 0; i <= NOT_CLASSIFIED; i++)
		counts[i] = 0;
}*/

/**
 * @fn  void Categorystats::add(category_t cat);
 * Increment the counter for the given category.
 * @param cat	Category to increment the counter.
 */

/**
 * @fn void CategoryStats::addLinked(void);
 * Add a new linked l-block to the statistics.
 */

/**
 * @fn int CategoryStats::get(category_t cat) const;
 * Get the counter of a category.
 * @param cat	Category to get counter for.
 * @return		Category count.
 */

/**
 * @fn int CategoryStats::total(void) const;
 * Get the total count of categories.
 * @return		Category total count.
 */

/**
 * @fn int CategoryStats::linked(void) const;
 * Get the count of linked statistics.
 * @return	Linked l-block statistics.
 */


/**
 * Put in the statistics to get statistics about cache categories.
 *
 * @par Hooks
 * @li processor configuration property list
 */
//Identifier<CategoryStats *> CATEGORY_STATS("otawa::CATEGORY_STATS", 0);


/**
 */
/*io::Output& operator<<(io::Output& out, const CategoryStats& stats) {
	static cstring names[] = {
			"invalid",
			"always-hit",
			"first-hit",
			"first-miss",
			"always-miss",
			"not-classified"
	};

	for(int i = ALWAYS_HIT; i <= NOT_CLASSIFIED; i++)
		out << names[i] << '\t' << (float(stats.get(category_t(i))) * 100 / stats.total())
			<< "% (" << stats.get(category_t(i)) << ")\n";
	out << "total\t\t100% (" << stats.total() << ")\n";
	out << "linked\t\t" << (float(stats.linked()) * 100 / stats.total())
		<< "% (" << stats.linked() << ")\n";
	return out;
}*/

} }	// otawa::dcache
