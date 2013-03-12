/*
 *	dcache::ACSBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

/**
 * @class MUSTProblem
 * The MUST problem provides the abstract interpretation of L1 data cache
 * for the MUST case. It implements the concept @ref AbstractDomain.
 */

/**
 * Build a MUST problem.
 * @param _size		Number of blocks.
 * @param _set		Number of sets in the cache.
 * @param _fw		Current workspace.
 * @param _cache	Analyzed cache.
 * @param _A		Associativity of the cache.
 */
MUSTProblem::MUSTProblem(int _size, int _set, WorkSpace *_fw, const hard::Cache *_cache, int _A)
:	fw(_fw),
	set(_set),
	cache(_cache),
	bot(_size, _A),
	ent(_size, _A),
	callstate(_size, _A)
{ ent.empty(); }


/**
 */
const MUSTProblem::Domain& MUSTProblem::bottom(void) const { return bot; }

/**
 */
const MUSTProblem::Domain& MUSTProblem::entry(void) const { return ent; }

/**
 */
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
				/* no break */
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


/**
 */
elm::io::Output& operator<<(elm::io::Output& output, const MUSTProblem::Domain& dom) {
	dom.print(output);
	return output;
}

SilentFeature::Maker<ACSBuilder> _maker;
/**
 * This feature ensures that the ACS (Abstract Cache State) for the MUST data cache analysis
 * has been built. Usually, the ACS are used to derivate the categories of each data cache access.
 *
 * @p Default Processor
 * @li @ref ACSBuilder
 *
 * @p Properties
 * @li @ref MUST_ACS
 *
 * @p Configuration
 * @li @ref ENTRY_MUST_ACS
 */
SilentFeature MUST_ACS_FEATURE("otawa::dcache::MUST_ACS_FEATURE", _maker);


/**
 * Provide for each basic block the ACS (Abstract Cache State) of the data cache
 * for each set of the cache.
 *
 * @p Hooks
 * @li @ref BasicBlock
 *
 * @p Feature
 * @li @ref MUST_ACS_FEATURE
 */
Identifier<genstruct::Vector<ACS *>* > MUST_ACS("otawa::dcache::MUST_ACS", 0);


/**
 * This configuration property is shared by all processor implementing the
 * @ref MUST_ACS_FEATURE. It allows to provide an initial ACS value
 * at the entry of the task. If not defined, the T (top) value is assumed.
 */
Identifier<Vector<ACS *>* > ENTRY_MUST_ACS("otawa::dcache::ENTRY_MUST_ACS", 0);


/**
 * This configuration property activates, if set to true (default), the pseudo-unrolling
 * for the cache analysis of @ref ACSBuilder. This option allows to get more precise
 * results but induces more computation time.
 */
Identifier<bool> DATA_PSEUDO_UNROLLING("otawa::dcache::PSEUDO_UNROLLING", true);


/**
 * According to its value, select the way the first-miss analysis is performed:
 * @li	DFML_INNER -- analysis of inner loop (average precision but costly in time),
		DFML_OUTER -- analysis of outer loop (very few precise)
		DFML_MULTI -- analysis of all loop level (very precise but costly)
		DFML_NONE -- no first miss analysis.
 */
Identifier<data_fmlevel_t> DATA_FIRSTMISS_LEVEL("otawa::dcache::FIRSTMISS_LEVEL", DFML_MULTI);


/**
 * @class ACSBuilder
 * This builder performs analysis of the L1 data cache and produces ACS for MUST and,
 * according to the configuration proerties, persistence. As a result, it provides ACS.
 *
 * @p Provided features
 * @li @ref MUST_ACS_FEATURE
 *
 * @p Required features
 * @li @ ref LOOP_INFO_FEATURE
 * @li @ref DATA_BLOCK_FEATURE
 *
 * @p configuration
 * @li @ref DATA_FIRSTMISS_LEVEL
 * @li @ref DATA_PSEUDO_UNROLLING
 */

p::declare ACSBuilder::reg = p::init("otawa::DataACSBuilder", Version(1, 0, 0))
	.base(Processor::reg)
	.maker<ACSBuilder>()
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(DATA_BLOCK_FEATURE)
	.provide(MUST_ACS_FEATURE);


/**
 */
ACSBuilder::ACSBuilder(p::declare& r): Processor(r), must_entry(0), unrolling(0), level(DFML_NONE) {
}


/**
 */
void ACSBuilder::processLBlockSet(WorkSpace *fw, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;
	int line = coll.cacheSet();

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
			MUSTProblem::Domain entry_dom(coll.count(), cache->wayCount());
			if(must_entry)
				entry_dom = *must_entry->get(line);
			mustHai.solve(0, &entry_dom);


			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++)
					MUST_ACS(bb)->set(line, new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));

		}
		else {
			DefaultListener<MUSTProblem> mustList(fw, mustProb);
			DefaultFixPoint<DefaultListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			MUSTProblem::Domain entry_dom(coll.count(), cache->wayCount());
			if(must_entry)
				entry_dom = *must_entry->get(line);
			mustHai.solve(0, &entry_dom);


			// Store the resulting ACS into the properties
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					ASSERT(line == coll.cacheSet());
					MUST_ACS(bb)->set(line, new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));
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


/**
 */
void ACSBuilder::configure(const PropList &props) {
	Processor::configure(props);
	level = DATA_FIRSTMISS_LEVEL(props);
	unrolling = DATA_PSEUDO_UNROLLING(props);
	must_entry = ENTRY_MUST_ACS(props);
}


/**
 */
void ACSBuilder::processWorkSpace(WorkSpace *fw) {
	//int i;
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	DATA_FIRSTMISS_LEVEL(fw) = level;

	// prepare the template of the final vector
	typedef genstruct::Vector<ACS *> acs_result_t;
	acs_result_t temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++)
		temp.add(0);

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
		for (CFG::BBIterator bb(cfg); bb; bb++)
			MUST_ACS(bb) = new acs_result_t(temp);

	// process block collections
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].cacheSet());
		processLBlockSet(fw, colls[i], cache);
	}

	// !!TODO!! code to do cleanup
}


/**
 * @class ACS
 * Representation of an Abstract Cache State where each data cache block
 * is represented by its age. The initial state is the unknown one, that
 * is when each block has an age of -1 (unknown age).
 */

/**
 * @fn ACS::ACS(const int _size, const int _A);
 * @param _size		Number of blocks.
 * @param _A		Cache associativity.
 */


/**
 * @fn int ACS::getSize(void);
 * Get the number of blocks of the ACS.
 * @return		Number of blocks.
 */


/**
 * @fn ACS::ACS(const ACS &source);
 * Constructor by cloning.
 * @param source	Cloned ACS.
 */


/**
 * @fn bool ACS::equals(const ACS& acs) const;
 * Test if two ACS are equals.
 * @param acs	ACS to compare the current one with.
 * @return		True if both ACS are equal, false else.
 */


/**
 * @fn void ACS::empty(void);
 * Set all block to age -1.
 */


/**
 * @fn bool ACS::contains(const int id) const;
 * Test if a block is in the ACS , that is, its each age is in the interval [0, A[
 * where A is the associativity of the cache.
 * @param id	Number of the tested block.
 * @return		True if it is in the cache, false else.
 */


/**
 * Print the ACS.
 * @param out	Stream to output to.
 */
void ACS::print(elm::io::Output &output) const {
	bool first = true;
	output << "[";
	for (int i = 0; i < size; i++) {
		if (age[i] != -1) {
			if (!first)
				output << ", ";
			output << i;
			output << ":";
			output << age[i];

			first = false;
		}
		output << "]";
	}
}


/**
 * @fn int ACS::getAge(int id) const;
 * Get the age of a block.
 * @param	Number of the block.
 * @return	Block age.
 */


/**
 * @fn void ACS::setAge(const int id, const int _age);
 * Change the age of a block.
 * @param id	Block number.
 * @param age	New age to set.
 */


/**
 * @fn void ACS::set(const ACS& acs);
 * Set the ages of the given ACS to the current one.
 * @param acs	ACS to set.
 */

} }	// otawa::dcache
