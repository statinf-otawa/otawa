/*
 * DataACSBuilder.cpp
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#include <elm/assert.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/hard/Cache.h>
#include <otawa/util/UnrollingListener.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/dcache/ACSBuilder.h>
#include <otawa/dcache/BlockBuilder.h>

namespace otawa { namespace dcache {

MUSTProblem::MUSTProblem(int _size, int _set, WorkSpace *_fw, const hard::Cache *_cache, int _A)
:	fw(_fw),
	set(_set),
	cache(_cache),
	bot(_size, _A),
	ent(_size, _A),
	callstate(_size, _A)
{ ent.empty(); }

const MUSTProblem::Domain& MUSTProblem::bottom(void) const { return bot; }
const MUSTProblem::Domain& MUSTProblem::entry(void) const { return ent; }

void MUSTProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	const Pair<int, BlockAccess *>& accesses = DATA_BLOCKS(bb);
	for(int i = 0; i < accesses.fst; i++) {
		BlockAccess& acc = accesses.snd[i];
		switch(acc.kind()) {
		case BlockAccess::RANGE:
			if(acc.first() < acc.last()) {
				if(set < acc.first() || set > acc.last())
					break;
			}
			else if( acc.first() < set || set < acc.last())
				break;
		case BlockAccess::ANY:
			out.ageAll();
			break;
		case BlockAccess::BLOCK:
			if(acc.block().set() == set)
				out.inject(acc.block().index());
			break;
		}
	}
}

elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom) {
	dom.print(output);
	return output;
}


Feature<ACSBuilder> DCACHE_ACS_FEATURE("otawa::DCACHE_ACS_FEATURE");
 Identifier<genstruct::Vector<MUSTProblem::Domain*>* > DCACHE_ACS_MUST("otawa::DCACHE_ACS_MUST", NULL);
 Identifier<Vector<MUSTProblem::Domain*>* > DCACHE_ACS_MUST_ENTRY("otawa::DCACHE_ACS_MUST_ENTRY", NULL);
//Identifier<genstruct::Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS("otawa::CACHE_ACS_PERS", NULL);
Identifier<bool> DATA_PSEUDO_UNROLLING("otawa::DATA_PSEUDO_UNROLLING", true);
Identifier<data_fmlevel_t> DATA_FIRSTMISS_LEVEL("otawa::DATA_FIRSTMISS_LEVEL", DFML_MULTI);

ACSBuilder::ACSBuilder(void) : Processor("otawa::DataACSBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(DATA_BLOCK_FEATURE);
	provide(DCACHE_ACS_FEATURE);
}


void ACSBuilder::processLBlockSet(WorkSpace *fw, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;
	int line = coll.set();

	/*
	 * Solve the problem for the current cache line:
	 * Now that the first/last lblock are detected, execute the analysis.
	 */

#ifdef DEBUG
	cout << "[TRACE] Doing line " << line << "\n";
#endif
	//if (level == FML_NONE) {
		// do only the MUST
		MUSTProblem mustProb(coll.count(), line, fw, cache, cache->wayCount());


		if (unrolling) {
			UnrollingListener<MUSTProblem> mustList(fw, mustProb);
			FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);


			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++)
					DCACHE_ACS_MUST(bb)->set(line, new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));

		}
		else {
			DefaultListener<MUSTProblem> mustList(fw, mustProb);
			DefaultFixPoint<DefaultListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);


			// Store the resulting ACS into the properties
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					ASSERT(line == coll.set());
					DCACHE_ACS_MUST(bb)->set(line, new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));
				}
		}
	/*} else {
		if (unrolling) {
			// Do combined MUST/PERS analysis
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			UnrollingListener<MUSTPERS> mustpersList( fw, mustpers);
			FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			// Store.
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must= mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers= mustpersList.results[cfg->number()][bb->number()]->getPers();
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(bb)->add(new PERSProblem::Domain(pers));

				}
		} else {

			// Do combined MUST/PERS analysis
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			DefaultListener<MUSTPERS> mustpersList( fw, mustpers);
			DefaultFixPoint<DefaultListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			// Store.
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must= mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers= mustpersList.results[cfg->number()][bb->number()]->getPers();
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(bb)->add(new PERSProblem::Domain(pers));
			}
		}
	}*/
}

void ACSBuilder::configure(const PropList &props) {
	Processor::configure(props);
	level = DATA_FIRSTMISS_LEVEL(props);
	unrolling = DATA_PSEUDO_UNROLLING(props);
	must_entry = DCACHE_ACS_MUST_ENTRY(props);
}

void ACSBuilder::processWorkSpace(WorkSpace *fw) {
	//int i;
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	DATA_FIRSTMISS_LEVEL(fw) = level;

	// prepare the template of the final vector
	typedef genstruct::Vector<MUSTProblem::Domain*> acs_result_t;
	acs_result_t temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++)
		temp.add(0);

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++) {
			DCACHE_ACS_MUST(bb) = new acs_result_t(temp);
			/*if (level != DFML_NONE)
				DCACHE_ACS_PERS(bb) = new genstruct::Vector<PERSProblem::Domain*>;*/
		}
	}

	// process block collections
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].set());
		processLBlockSet(fw, colls[i], cache);
	}

	// !!TODO!! code to do cleanup
}

} }	// otawa::dcache
