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

namespace otawa { namespace dcache {

MAYProblem::MAYProblem(
	const BlockCollection& collection,
	WorkSpace *_fw,
	const hard::Cache *_cache)
:	coll(collection),
	fw(_fw),
	line(collection.set()),
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


void MAYProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
    assign(out, in);
	const Pair<int, BlockAccess *>& accesses = DATA_BLOCKS(bb);
	for(int i = 0; i < accesses.fst; i++) {
		BlockAccess& acc = accesses.snd[i];
		switch(acc.kind()) {
		case BlockAccess::RANGE:
		case BlockAccess::ANY:
			// !!TODO!! valid this with Clément
			break;
		case BlockAccess::BLOCK:
			if(acc.block().set() == line)
				out.inject(acc.block().index());
			break;
		}
	}
}

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom) {
	dom.print(output);
	return output;
}

Feature<ACSMayBuilder> ACS_MAY_FEATURE("otawa::dcache::ACS_MAY_FEATURE");
Identifier<may_acs_t *> ACS_MAY("otawa::dcache::ACS_MAY", NULL);
Identifier<may_acs_t *> ACS_MAY_ENTRY("otawa::dcache::ACS_MAY_ENTRY", NULL);


ACSMayBuilder::ACSMayBuilder(void) : Processor("otawa::dcache::ACSMayBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(DATA_BLOCK_FEATURE);
	provide(ACS_MAY_FEATURE);
}


void ACSMayBuilder::processLBlockSet(WorkSpace *fw, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;
	int line = coll.set();

#ifdef DEBUG
	cout << "[TRACE] Doing line " << line << "\n";
#endif
	MAYProblem mayProb(coll, fw, cache);
	if (unrolling) {
		UnrollingListener<MAYProblem> mayList(fw, mayProb);
		FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > > mayHai(mayFp, *fw);
		mayHai.solve(NULL, may_entry ? may_entry->get(line) : NULL);
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				ACS_MAY(bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));

	}
	else {
		DefaultListener<MAYProblem> mayList(fw, mayProb);
		DefaultFixPoint<DefaultListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<DefaultFixPoint<DefaultListener<MAYProblem> > > mayHai(mayFp, *fw);
		mayHai.solve(NULL, may_entry ? may_entry->get(line) : NULL);
		/* Store the resulting ACS into the properties */
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				ACS_MAY(bb)->set(line, new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));
	}
}

void ACSMayBuilder::configure(const PropList &props) {
	Processor::configure(props);
	unrolling = DATA_PSEUDO_UNROLLING(props);
	may_entry = ACS_MAY_ENTRY(props);
}

void ACSMayBuilder::processWorkSpace(WorkSpace *fw) {
	const hard::Cache *cache = fw->platform()->cache().dataCache();

	// prepare the template of the final vector
	typedef genstruct::Vector<MAYProblem::Domain*> acs_result_t;
	acs_result_t temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++)
		temp.add(0);

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++)
			ACS_MAY(bb) = new acs_result_t(temp);
	}

	// process block collections
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].set());
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
