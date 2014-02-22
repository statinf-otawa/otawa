/*
 * dcache_ACSMayBuilder.cpp
 *
 *  Created on: 15 juil. 2009
 *      Author: casse
 */

#include <otawa/dcache/ACSMayBuilder.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/dcache/BlockBuilder.h>
#include <otawa/util/UnrollingListener.h>
#include <otawa/util/DefaultFixPoint.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/dcache/ACSBuilder.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa { namespace dcache {

MAYProblem::MAYProblem(
	const BlockCollection& collection,
	WorkSpace *_fw,
	const hard::Cache *_cache)
:	coll(collection),
	fw(_fw),
	line(collection.cacheSet()),
	cache(_cache),
	bot(collection.count(), cache->wayCount()),
	ent(collection.count(), cache->wayCount()),
	callstate(collection.count(), cache->wayCount())
{
		ent.empty();
}


MAYProblem::~MAYProblem(void) {
}


const MAYProblem::Domain& MAYProblem::bottom(void) const {
	return bot;
}
const MAYProblem::Domain& MAYProblem::entry(void) const {
	return ent;
}


/**
 * Update the state according to the given block access.
 * @param s			State to update.
 * @param access	Cache access.
 */
void MAYProblem::update(Domain& s, const BlockAccess& access) {
	switch(access.kind()) {
	case BlockAccess::RANGE:
	case BlockAccess::ANY:
		break;
	case BlockAccess::BLOCK:
		if(access.block().set() == line)
			s.inject(access.block().index());
		break;
	}
}


/**
 * Produces the output MAY cache state with the execution of given BB.
 * @param out	Output state.
 * @param in	Input state.
 * @param bb	BB to to analyze.
 */
void MAYProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
    assign(out, in);
	const Pair<int, BlockAccess *>& accesses = DATA_BLOCKS(bb);
	for(int i = 0; i < accesses.fst; i++) {
		BlockAccess& acc = accesses.snd[i];
		update(out, acc);
	}
}

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom) {
	dom.print(output);
	return output;
}

static SilentFeature::Maker<ACSMayBuilder> maker;
/**
 * This feature that the MAY analysis has been performed for the L1 data cache
 * and that the ACS are provided at the entry of each basic block.
 *
 * @p Default processor
 * @li @ref ACSMayBuilder
 *
 * @p Properties
 * @li @ref MAY_ACS
 *
 * @p Configuration
 * @li @ref ENTRY_MAY_ACS
 * @ingroup dcache
 */
SilentFeature MAY_ACS_FEATURE("otawa::dcache::MAY_ACS_FEATURE", maker);


/**
 * Provide the ACS for the MAY analysis. The vector contains one line for each cache set.
 *
 * @p Hook
 * @li @ref BasicBlock
 * @ingroup dcache
 */
Identifier<Vector<ACS *> *> MAY_ACS("otawa::dcache::MAY_ACS", 0);


/**
 * Configuration property giving the ACS at the startup of the task.
 * The vector contains one ACS for each cache set.
 * @ingroup dcache
 */
Identifier<Vector<ACS *> *> ENTRY_MAY_ACS("otawa::dcache::ENTRY_MAY_ACS", 0);


/**
 * @class ACSMayBuilder
 * This processor computes the ACS for the MAY cache analysis.
 *
 * @p Provided features
 * @li @ref ACS_MAY_FEATURE
 *
 * @p Required features
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref DATA_BLOCK_FEATURE
 *
 * @p Configuration
 * @li @ref ENTRY_MAY_ACS
 * @ingroup dcache
 */

p::declare ACSMayBuilder::reg = p::init("otawa::dcache::ACSMayBuilder", Version(1, 0, 0))
	.maker<ACSMayBuilder>()
	.base(Processor::reg)
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(DATA_BLOCK_FEATURE)
	.provide(MAY_ACS_FEATURE);


/**
 */
ACSMayBuilder::ACSMayBuilder(p::declare& r): Processor(r), unrolling(false), may_entry(0) {
}


/**
 */
void ACSMayBuilder::processLBlockSet(WorkSpace *fw, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;
	int line = coll.cacheSet();

#ifdef DEBUG
	cout << "[TRACE] Doing line " << line << "\n";
#endif
	MAYProblem mayProb(coll, fw, cache);
	if (unrolling) {
		UnrollingListener<MAYProblem> mayList(fw, mayProb);
		FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > > mayHai(mayFp, *fw);
		MAYProblem::Domain entry_dom(coll.count(), cache->wayCount());
		if(may_entry)
			entry_dom = *may_entry->get(line);
		mayHai.solve(0, &entry_dom);
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				MAY_ACS(bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));

	}
	else {
		DefaultListener<MAYProblem> mayList(fw, mayProb);
		DefaultFixPoint<DefaultListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<DefaultFixPoint<DefaultListener<MAYProblem> > > mayHai(mayFp, *fw);
		MAYProblem::Domain entry_dom(coll.count(), cache->wayCount());
		if(may_entry)
			entry_dom = *may_entry->get(line);
		mayHai.solve(0, &entry_dom);
		/* Store the resulting ACS into the properties */
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				MAY_ACS(bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));
	}
}

/**
 */
void ACSMayBuilder::configure(const PropList &props) {
	Processor::configure(props);
	unrolling = DATA_PSEUDO_UNROLLING(props);
	may_entry = ENTRY_MAY_ACS(props);
}

/**
 */
void ACSMayBuilder::processWorkSpace(WorkSpace *fw) {
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	// prepare the template of the final vector
	typedef genstruct::Vector<ACS *> acs_result_t;
	acs_result_t temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++)
		temp.add(0);

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++)
			MAY_ACS(bb) = new acs_result_t(temp);
	}

	// process block collections
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].cacheSet());
		processLBlockSet(fw, colls[i], cache);
	}

	// !!TODO!! code to do cleanup
	/* for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
		for (CFG::BBIterator bb(cfg); bb; bb++)
			ACS_MAY(bb) = new genstruct::Vector<MAYProblem::Domain*>;

	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);

	for (int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, colls[i], cache); */
}


} } // otawa::dcache
