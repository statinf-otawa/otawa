/*
 * dcache_ACSMayBuilder.cpp
 *
 *  Created on: 15 juil. 2009
 *      Author: casse
 */

#include <elm/data/Vector.h>
#include <otawa/dcache/ACSMayBuilder.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/dcache/BlockBuilder.h>
#include <otawa/dfa/hai/UnrollingListener.h>
#include <otawa/dfa/hai/DefaultFixPoint.h>
#include <otawa/dfa/hai/DefaultListener.h>
#include <otawa/dcache/ACSBuilder.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa { namespace dcache {

using namespace otawa::dfa::hai;

MAYProblem::MAYProblem(
	const BlockCollection& collection,
	WorkSpace *_fw,
	const hard::Cache *_cache)
:	callstate(collection.count(), _cache->wayCount()),
	coll(collection),
	fw(_fw),
	set(collection.cacheSet()),
	cache(_cache),
	_top(collection.count(), _cache->wayCount()),
	bot(collection.count(), _cache->wayCount()),
	_entry(collection.count(), _cache->wayCount(), 0)
{
		_top.empty();
}


MAYProblem::~MAYProblem(void) {
}


/**
 * Update the state according to the given block access.
 * @param s			State to update.
 * @param access	Cache access.
 */
void MAYProblem::update(Domain& s, const BlockAccess& access) {
	switch(access.action ()) {

	case BlockAccess::LOAD:
		switch(access.kind()) {
		case BlockAccess::ANY:
			s.refreshAll();
			break;
		case BlockAccess::BLOCK: {
			if(access.block().set() == set)
				s.inject(access.block().index());
			}
			break;
		case BlockAccess::RANGE: {
				auto b = access.blockIn(set);
				if(b != nullptr)
					s.set(b->index(), 0);
			}
			break;
		}
		break;

	case BlockAccess::STORE:
		if(cache->writePolicy() == hard::Cache::WRITE_THROUGH)
			switch(access.kind()) {
			case BlockAccess::ANY:
				s.refreshAllWriteThrough(-1);
				break;
			case BlockAccess::BLOCK:
				if(access.block().set() == set)
					s.injectWriteThrough(access.block().index());
				break;
			case BlockAccess::RANGE: {
					auto b = access.blockIn(set);
					if(b != nullptr)
						s.set(b->index(), 0);
				}
				break;
			}
		else if(cache->writePolicy() == hard::Cache::WRITE_BACK)
			switch(access.kind()){
			case BlockAccess::ANY:
				s.refreshAll();
				break;
			case BlockAccess::BLOCK:
				if(access.block().set() == set)
					s.inject(access.block().index());
				break;
			case BlockAccess::RANGE: {
					auto b = access.blockIn(set);
					if(b != nullptr)
						s.set(b->index(), 0);
				}
				break;
			}
		else {
			ASSERTP(false, "unsupported replacement policy");
		}
		break;

	default:
		ASSERTP(false, "unsupported access action");
		break;
	}
}


/**
 * Produces the output MAY cache state with the execution of given BB.
 * @param out	Output state.
 * @param in	Input state.
 * @param bb	BB to to analyze.
 */
void MAYProblem::update(Domain& out, const Domain& in, otawa::Block* bb) {
    assign(out, in);
	for(const auto& acc: *DATA_BLOCKS(bb))
		update(out, acc);
}

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom) {
	dom.print(output);
	return output;
}


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
p::feature MAY_ACS_FEATURE("otawa::dcache::MAY_ACS_FEATURE", new Maker<ACSMayBuilder>());


/**
 * Provide the ACS for the MAY analysis. The vector contains one line for each cache set.
 *
 * @p Hook
 * @li @ref BasicBlock
 * @ingroup dcache
 */
p::id<Vector<ACS *> *> MAY_ACS("otawa::dcache::MAY_ACS", 0);


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
	MAYProblem mayProb(coll, fw, cache);
	if (unrolling) {
		UnrollingListener<MAYProblem> mayList(fw, mayProb);
		FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > mayFp(mayList);
		HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > > mayHai(mayFp, *fw);
		MAYProblem::Domain entry_dom(coll.count(), cache->wayCount());
		if(may_entry)
			entry_dom = *may_entry->get(line);
		mayHai.solve(0, &entry_dom);
		for(CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
			for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
				MAY_ACS(*bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->index()][bb->index()]));

	}
	else {
		DefaultListener<MAYProblem> mayList(fw, mayProb);
		DefaultFixPoint<DefaultListener<MAYProblem> > mayFp(mayList);
		HalfAbsInt<DefaultFixPoint<DefaultListener<MAYProblem> > > mayHai(mayFp, *fw);
		MAYProblem::Domain entry_dom(coll.count(), cache->wayCount());
		if(may_entry)
			entry_dom = *may_entry->get(line);
#ifdef OLD_IMPLEMENTATION
#else
		else
			entry_dom = mayProb.entry();
#endif
		mayHai.solve(0, &entry_dom);
		/* Store the resulting ACS into the properties */
		for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
			for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
				MAY_ACS(*bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->index()][bb->index()]));
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
	const hard::Cache *cache = hard::CACHE_CONFIGURATION_FEATURE.get(fw)->dataCache();

	// prepare the template of the final vector
	typedef Vector<ACS *> acs_result_t;
	acs_result_t temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++)
		temp.add(0);

	// Build the vectors for receiving the ACS...
	// Now each block is attributed with MAY_ACS, i.e. MAY_ACS(bb) will never be NULL/0 (the default value of MAY_ACS)
	for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++) {
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
			MAY_ACS(*bb) = new acs_result_t(temp);
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
			ACS_MAY(bb) = new Vector<MAYProblem::Domain*>;

	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);

	for (int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, colls[i], cache); */
}


} } // otawa::dcache
