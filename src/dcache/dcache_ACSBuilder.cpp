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

//#define HAI_DEBUG

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
#include <otawa/dcache/MUSTPERS.h>

#define MUST_DEBUG(x)	//cerr << x << io::endl

namespace otawa { namespace dcache {

/**
 * @class MUSTProblem
 * The MUST problem provides the abstract interpretation of L1 data cache
 * for the MUST case. It implements the concept @ref AbstractDomain.
 * @ingroup dcache
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
	callstate(_size, _A),
	size(_size)
{ ent.empty(); }


/**
 */
const MUSTProblem::Domain& MUSTProblem::bottom(void) const { return bot; }

/**
 */
const MUSTProblem::Domain& MUSTProblem::entry(void) const { return ent; }


/**
 * Perform a purge according to the given block access.
 * @param out	In-out domain.
 * @param acc	Purging block access.
 */
void MUSTProblem::purge(Domain& out, const BlockAccess& acc) {
  ASSERT(acc.action() == BlockAccess::PURGE);
  //ASSERT(acc.kind() <= BlockAccess::RANGE);

	switch(acc.kind()) {
	case BlockAccess::RANGE:
		if(!acc.inRange(set))
			break;
		// no break
	case BlockAccess::ANY:
		out.empty();
		break;
	case BlockAccess::BLOCK:
		if(set == acc.block().set())
			out[acc.block().index()] = -1;
		break;
	}
}


/**
 * Update for an access.
 */
void MUSTProblem::update(Domain& s, const BlockAccess& access) {
	ASSERT(access.action() <= BlockAccess::PURGE);
	ASSERT(access.kind() <= BlockAccess::RANGE);
	MUST_DEBUG("\t\t\tupdating with " << acc);
	switch(access.action()) {

	case BlockAccess::LOAD:
	case BlockAccess::STORE:
		switch(access.kind()) {
		case BlockAccess::RANGE:
			if(access.first() < access.last()) {
				if(set < access.first() || set > access.last())
					break;
			}
			else if( access.first() < set || set < access.last())
				break;
				/* no break */
		case BlockAccess::ANY:
			s.ageAll();
			break;
		case BlockAccess::BLOCK:
			if(access.block().set() == set)
				s.inject(access.block().index());
			break;
		}
		break;

	case BlockAccess::PURGE:
		purge(s, access);
		break;
	}

}

/**
 */
void MUSTProblem::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	const Pair<int, BlockAccess *>& accesses = DATA_BLOCKS(bb);
	for(int i = 0; i < accesses.fst; i++) {
		BlockAccess& acc = accesses.snd[i];
		update(out, acc);
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
 * @ingroup dcache
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
 * @ingroup dcache
 */
Identifier<genstruct::Vector<ACS *>* > MUST_ACS("otawa::dcache::MUST_ACS", 0);


/**
 * This configuration property is shared by all processor implementing the
 * @ref MUST_ACS_FEATURE. It allows to provide an initial ACS value
 * at the entry of the task. If not defined, the T (top) value is assumed.
 * @ingroup dcache
 */
Identifier<Vector<ACS *>* > ENTRY_MUST_ACS("otawa::dcache::ENTRY_MUST_ACS", 0);


/**
 * This configuration property activates, if set to true (default), the pseudo-unrolling
 * for the cache analysis of @ref ACSBuilder. This option allows to get more precise
 * results but induces more computation time.
 * @ingroup dcache
 */
Identifier<bool> DATA_PSEUDO_UNROLLING("otawa::dcache::PSEUDO_UNROLLING", false);


/**
 * According to its value, select the way the first-miss analysis is performed:
 * @liDFML_INNER -- analysis of inner loop (average precision but costly in time),
 * @li DFML_OUTER -- analysis of outer loop (very few precise)
 * @li DFML_MULTI -- analysis of all loop level (very precise but costly)
 * @li DFML_NONE -- no first miss analysis.
 * @ingroup dcache
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
 * @ingroup dcache
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

	if(logFor(LOG_FILE))
		log << "\tSET " << coll.cacheSet() << io::endl;

	// without persistence
	if (level == DFML_NONE) {
		// do only the MUST
		MUSTProblem mustProb(coll.count(), line, fw, cache, cache->wayCount());

		// computation with unrolling
		if (unrolling) {
			UnrollingListener<MUSTProblem> mustList(fw, mustProb);
			FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > > mustHai(mustFp, *fw);
			MUSTProblem::Domain entry_dom = mustProb.bottom();
			if(must_entry)
				entry_dom = *must_entry->get(line);
			mustHai.solve(0, &entry_dom);


			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
				if(logFor(LOG_CFG))
					log << "\t\tCFG " << *cfg << io::endl;
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUST_ACS(bb)->set(line, new ACS(*mustList.results[cfg->number()][bb->number()]));
					if(logFor(LOG_BLOCK)) {
						log << "\t\t\t" << *bb << ": " << *(MUST_ACS(bb)->get(line)) << io::endl;
						//log << "\t\t\t" << *bb << ": " << *mustList.results[cfg->number()][bb->number()] << io::endl;
					}
				}
			}

		}

		// computation without unrolling
		else {
			DefaultListener<MUSTProblem> mustList(fw, mustProb);
			DefaultFixPoint<DefaultListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			MUSTProblem::Domain entry_dom  = mustProb.entry();
			if(must_entry) {
				entry_dom = *must_entry->get(line);
				if(logFor(LOG_CFG))
					log << "\t\tusing user entry: " << *must_entry->get(line) << io::endl;
			}
			if(logFor(LOG_CFG))
				log << "\t\tentry = " << entry_dom << io::endl;
			mustHai.solve(0, &entry_dom);

			// Store the resulting ACS into the properties
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
				if(logFor(LOG_CFG))
					log << "\t\tCFG " << *cfg << io::endl;
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					ASSERT(line == coll.cacheSet());
					MUST_ACS(bb)->set(line, new ACS(*mustList.results[cfg->number()][bb->number()]));
					if(logFor(LOG_BLOCK))
						log << "\t\t\t" << *bb << ": " << *(MUST_ACS(bb)->get(line)) << io::endl;
				}
			}
		}

	// with persistence
	}
	else {

		// with unrolling
		if (unrolling) {
			// Do combined MUST/PERS analysis
			MUSTPERS mustpers(&coll, fw, cache);
			UnrollingListener<MUSTPERS> mustpersList( fw, mustpers);
			FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			// Store.
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
				if(logFor(LOG_CFG))
					log << "\t\tCFG " << *cfg << io::endl;
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must = mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers = mustpersList.results[cfg->number()][bb->number()]->getPers();
					MUST_ACS(bb)->set(line, new ACS(must));
					if(logFor(LOG_BLOCK))
						log << "\t\t\t" << *bb << ": " << *(MUST_ACS(bb)->get(line)) << io::endl;
					PERS_ACS(bb)->set(line, new ACS(pers.getWhole()));
					acs_stack_t& stack = LEVEL_PERS_ACS(bb)->item(line);
					int len = pers.length();
					stack.allocate(len);
					for(int i = 0; i < len; i++)
						stack[i] = new ACS(pers.getItem(i));
				}
			}
		}

		// without unrolling
		else {

			// Do combined MUST/PERS analysis
			MUSTPERS mustpers(&coll, fw, cache);
			DefaultListener<MUSTPERS> mustpersList( fw, mustpers);
			DefaultFixPoint<DefaultListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			// Store.
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
				if(logFor(LOG_CFG))
					log << "\t\tCFG " << *cfg << io::endl;
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must= mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers= mustpersList.results[cfg->number()][bb->number()]->getPers();
					MUST_ACS(bb)->set(line, new ACS(must));
					if(logFor(LOG_BLOCK))
						log << "\t\t\t" << *bb << ": " << *(MUST_ACS(bb)->get(line)) << io::endl;
					PERS_ACS(bb)->set(line, new ACS(pers.getWhole()));
					acs_stack_t& stack = LEVEL_PERS_ACS(bb)->item(line);
					int len = pers.length();
					stack.allocate(len);
					for(int i = 0; i < len; i++)
						stack[i] = new ACS(pers.getItem(i));
				}
			}
		}
	}
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
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	DATA_FIRSTMISS_LEVEL(fw) = level;

	// prepare the template of the final vector
	typedef genstruct::Vector<ACS *> acs_result_t;
	acs_result_t temp(cache->rowCount());
	typedef acs_stack_table_t level_acs_t;
	level_acs_t level_temp(cache->rowCount());
	for(int i = 0; i < cache->rowCount(); i++) {
		temp.add(0);
		level_temp.add(0);
	}

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
		for (CFG::BBIterator bb(cfg); bb; bb++) {
			MUST_ACS(bb) = new acs_result_t(temp);
			if(level != DFML_NONE) {
				PERS_ACS(bb) = new acs_result_t(temp);
				LEVEL_PERS_ACS(bb) = new acs_stack_table_t(level_temp);
			}
		}

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
 * @ingroup dcache
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
			output << ": ";
			output << age[i];

			first = false;
		}
	}
	output << "]";
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


/**
 * @class PERSProblem
 *
 * Problem for computing the PERS ACS of L-blocks.
 * This implements Ferdinand's Persistence analysis.
 * @ingroup dcache
 */



PERSProblem::PERSProblem(const int _size, const BlockCollection *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A)
:	callstate(_size, _A),
	lbset(_lbset),
	fw(_fw),
	cache(_cache),
	bot(_size, _A),
	ent(_size, _A),
	line(lbset->cacheSet()),
	cfg(0)
{
		bot.setToBottom();
		ent.empty();
#ifdef PERFTEST
		bot.enterContext();
		ent.enterContext();
#endif

}

PERSProblem::~PERSProblem() {

}
const PERSProblem::Domain& PERSProblem::bottom(void) const {
		return bot;
}

const PERSProblem::Domain& PERSProblem::entry(void) const {
		return ent;
}

void PERSProblem::update(Domain& out, const Domain& in, BasicBlock* bb)  {
		cerr << "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.\n";
		ASSERT(false);
}


/**
 * Apply the given purge action to the given item.
 * @param item	Item to work on.
 * @param acc	Purge action to perform.
 */
void PERSProblem::purge(Item& item, const BlockAccess& acc) {
	ASSERT(acc.action() == BlockAccess::PURGE);
	ASSERT(acc.kind() <= BlockAccess::RANGE);
	switch(acc.kind()) {
	case BlockAccess::RANGE:
		if(!acc.inRange(line))
			break;
		// no break
	case BlockAccess::ANY:
		for(int i = 0; i < item.getSize(); i++)
			if(item[i] != -1)
				item[i] = item.getA();
		break;
	case BlockAccess::BLOCK:
		if(acc.block().set() == line) {
			if(item[acc.block().set()] != -1)
				item[acc.block().set()] = item.getA();
		}
		break;
	}
}


/**
 * Apply the given purge action to the given domain.
 * @param domain	Domain to work on.
 * @param acc		Purge action to perform.
 */
void PERSProblem::purge(Domain& domain, const BlockAccess& acc) {
	purge(domain.getWhole(), acc);
	for(int i = 0; i < domain.length(); i++)
		purge(domain.getItem(i), acc);
}


elm::io::Output& operator<<(elm::io::Output& output, const PERSProblem::Domain& dom) {
	dom.print(output);
	return output;
}


/**
 * Consider that an unknown access is performed and, therefore,
 * all blocks must be aged.
 */
void PERSProblem::Item::ageAll(void) {
	for (int i = 0; i < size; i++)
		if (age[i] >= 0 && age[i] < A)
			age[i]++;
}


/**
 * Consider that an unknown access is performed and, therefore,
 * all blocks must be aged.
 */
void PERSProblem::Domain::ageAll(void) {
	whole.ageAll();
	for(int i = 0; i < data.count(); i++)
		data[i]->ageAll();
}


/**
 * @class MUSTPER
 * Describes the ACS used in combined MUST-persistence problem.
 * This combination slightly improve the speed of the analysis but also allows to work-around
 * a bug of the original persistence analysis.
 * @ingroup dcache
 */


/**
 * @class MUSTPERS::Domain
 * ACS of the MUSTPERS problem.
 */


/**
 * Print the MUSTPERS ACS.
 * @param output	Stream to output to.
 */
void MUSTPERS::print(elm::io::Output &output, const Domain& d) const {
	output << "PERS=[ ";
	d.pers.print(output);
	output << "] MUST=[ ";
	d.must.print(output);
	output << "]";
}


MUSTPERS::MUSTPERS(const BlockCollection *_lbset, WorkSpace *_fw, const hard::Cache *_cache)
:	mustProb(_lbset->count(), _lbset->cacheSet(), _fw, _cache, _cache->wayCount()),
	persProb(_lbset->count(), _lbset, _fw, _cache, _cache->wayCount()),
	bot(_lbset->count(),  _cache->wayCount()),
	ent(_lbset->count(),  _cache->wayCount()),
	set(_lbset->cacheSet())
{

		persProb.assign(bot.pers, persProb.bottom());
		mustProb.assign(bot.must, mustProb.bottom());

		persProb.assign(ent.pers, persProb.entry());
		mustProb.assign(ent.must, mustProb.entry());
}


const MUSTPERS::Domain& MUSTPERS::bottom(void) const {
		return bot;
}
const MUSTPERS::Domain& MUSTPERS::entry(void) const {
		return ent;
}


/**
 * Update according to the given access.
 * @param s			Domain to udpate.
 * @param access	Access to apply.
 */
void MUSTPERS::update(Domain& s, const BlockAccess& access) {
	MUST_DEBUG("\t\t\tupdating with " << acc);
	switch(access.action()) {

	case BlockAccess::LOAD:
	case BlockAccess::STORE:
		switch(access.kind()) {
		case BlockAccess::RANGE:
			if(access.first() < access.last()) {
				if(set < access.first() || set > access.last())
					break;
			}
			else if(access.first() < set || set < access.last())
				break;
				/* no break */
		case BlockAccess::ANY:
			ageAll(s);
			break;
		case BlockAccess::BLOCK:
			if(access.block().set() == set)
				inject(s, access.block().index());
			break;
		}
		break;

	case BlockAccess::PURGE:
		mustProb.purge(s.must, access);
		persProb.purge(s.pers, access);
		break;

	default:
		ASSERTP(false, "bad block access action: " << access.kind());
		break;
	}
}


/**
 */
void MUSTPERS::update(Domain& out, const Domain& in, BasicBlock* bb) {
	assign(out, in);
	const Pair<int, BlockAccess *>& accesses = DATA_BLOCKS(bb);
	for(int i = 0; i < accesses.fst; i++) {
		BlockAccess& acc = accesses.snd[i];
		update(out, acc);
	}
}


/**
 * @class PERSProblem
 * Problem for the data cache persistence analysis.
 * @ingroup dcache
 */


/**
 * @class PERSProblem::Item
 * ACS for the multi-level data cache persistence analysis. Items are composed on each level.
 */


/**
 * Perform join on persistent ACS.
 * @param	dom		Domain to join with.
 */
void PERSProblem::Item::lub(const PERSProblem::Item &dom) {
	ASSERT((A == dom.A) && (size == dom.size));
	for (int i = 0; i < size; i++)
		if ((age[i] == -1) || ((age[i] < dom.age[i]) && (dom.age[i] != -1)) )
			age[i] = dom.age[i];
}


/**
 * Test if two ACS are equals.
 * @param dom	ACS to test with.
 * @return		True if they are equals, false else.
 */
bool PERSProblem::Item::equals(const Item &dom) const {
	ASSERT((A == dom.A) && (size == dom.size));
	for (int i = 0; i < size; i++)
		if (age[i] != dom.age[i])
			return false;
	return true;
}


/**
 * Consider that an access to the designed block is performed.
 * @param must		Must ACS (useful to work around a bug in the initial persistent ACS implementation).
 * @param id		Identifier of the injected block.
 */
void PERSProblem::Item::inject(MUSTProblem::Domain *must, const int id) {
	if (must->contains(id)) {
		for (int i = 0; i < size; i++)
			if ((age[i] < age[id]) && (age[i] != -1) && (age[i] != A))
				age[i]++;
		age[id] = 0;
	}
	else
		for (int i = 0; i < size; i++)
			if ((age[i] != -1) && (age[i] != A))
				age[i]++;
	age[id] = 0;
}


/**
 * Change the age of the designed block considering the given damage to the ACS.
 * @param id		Block identifier.
 * @param damage	Number of aging to apply.
 */
void PERSProblem::Item::addDamage(const int id, int damage) {
	ASSERT((id >= 0) && (id < size));
	if (age[id] == -1)
		return;
	age[id] += damage;
	if (age[id] > A)
		age[id] = A;
}


/**
 * Assignement of persistence analysis ACS.
 * @param src	Assigning domain.
 * @return		Assigned domain.
 */
PERSProblem::Domain& PERSProblem::Domain::operator=(const Domain &src) {
	if (src.isBottom)
		setToBottom();
	else {
		whole = src.whole;
		isBottom = false;

		// assign ACS whose place is already available
		int sdl = src.data.length();
		int dl = data.length();
		int minl = (sdl > dl) ? dl : sdl;
		for (int i = 0; i < minl; i++)
			*data[i] = *src.data[i];

		// extend the vector and add the missing ACS
		// TODO	if the target size is smaller that the source one, some ACS needs to be deleted before the vector size reduction
		data.setLength((sdl > dl) ? dl : sdl);
		for (int i = dl; i < sdl; i++)
			data.add(new Item(*src.data[i]));
	}
	return *this;
}


/**
 * Compute the join of the current value with the given one
 * and the result is left in the current ACS.
 * @param dom	ACS to join with.
 */
void PERSProblem::Domain::lub(const Domain &dom) {

	// 1. lub(anything, bottom) == anything
	if(dom.isBottom)
		return;
	if(isBottom) {
		for (int i = 0; i < dom.data.length(); i++)
			data.add(new Item(*dom.data[i]));
		whole = dom.whole;
		isBottom = false;
		return;
	}

	 // 2. lub(dom1,dom2) where dom1 and dom2 has the same size, do the LUB of each corresponding item.
	 // 3. lub(dom1,dom2) where dom1 has less items than dom2: we discard  items of dom2 (starting from outer-most loop)
	// until it has the same size as dom1, then apply rule 2.
	int dl = data.length();
	int ddl = dom.data.length();
	int length = (dl < ddl) ? dl : ddl;
	for (int i = dl - 1, j = ddl - 1, k = 0; k < length; i--, j--, k++)
		data[i]->lub(*dom.data[j]);
	for (int i = 0; i < dl - length; i++)
		data.remove(0);
	whole.lub(dom.whole);
}




/**
 * Special LUB: do the lub of each Item in current domain with
 * item passed as parameter. Used for the partial analysis
 */
void PERSProblem::Domain::lub(const Item &item) {
	 for (int i = 0; i < data.length(); i++)
	 	data[i]->lub(item);
	 whole.lub(item);
}


/**
 * Test if two ACS are equals.
 * @param dom	ACS to compare with.
 * @return		True if they are equal, false else.
 */
bool PERSProblem::Domain::equals(const Domain &dom) const {
	ASSERT(!isBottom && !dom.isBottom);
	for (int i = 0; i < dom.data.length(); i++)
		if (!data[i]->equals(*dom.data[i]))
			return false;
	return (whole.equals(dom.whole) && (isBottom == dom.isBottom));
}


/**
 * Empty the domain.
 */
void PERSProblem::Domain::empty(void) {
	for (int i = 0; i < data.length(); i++)
		delete data[i];
	data.clear();
	whole.empty();
	isBottom = false;
}


/**
 * Set the bottom value in the ACS.
 */
void PERSProblem::Domain::setToBottom(void) {
	for (int i = 0; i < data.length(); i++)
		delete data[i];
	data.clear();
	whole.empty();
	isBottom = true;
}


/**
 * Consider that the designed block is accessed.
 * @param must	Must ACS (useful to work-around a bug in the original persistence analysis).
 * @param id	Identity of the accessed block.
 */
void PERSProblem::Domain::inject(MUSTProblem::Domain *must, const int id) {
	ASSERT(!isBottom);
	for (int i = 0; i < data.length(); i++)
		data[i]->inject(must, id);
	whole.inject(must, id);
}


/**
 * In partial analysis, add age damage to the given block.
 * @param id		Identity of the aged block.
 * @param damage	Damage value.
 */
void PERSProblem::Domain::addDamage(const int id, int damage) {
	ASSERT(!isBottom);
	for (int n = 0; n < data.length(); n++)
		data[n]->addDamage(id, damage);
	whole.addDamage(id, damage);
}


/**
 * Refresh a block to the given age (if required).
 * @param id		Refreshed block.
 * @param newage	Maximal new age of the block.
 */
void PERSProblem::Domain::refresh(const int id, int newage) {
	ASSERT(!isBottom);
	for (int n = 0; n < data.length(); n++)
		data[n]->refresh(id, newage);
	whole.refresh(id, newage);
}


/**
 * Display the ACS.
 * @param output	Stream to output to.
 */
void PERSProblem::Domain::print(elm::io::Output &output) const {
	bool first = true;
	if (isBottom) {
		output << "BOTTOM";
		return;
	}
	output << "(W=";
	whole.print(output);
	output << ", ";
	for (int i = 0; i < data.length(); i++) {
		if (!first)
			output << "|";
		data[i]->print(output);
		first = false;
	}
	output << ")";
}


/**
 * Called when a loop context is entered to add a level.
 */
void PERSProblem::Domain::enterContext(void) {
	ASSERT(!isBottom);
	Item item(whole.getSize(), whole.getA());
	item.empty();
	data.push(new Item(item));

}


/**
 * Called when a loop context is left to remove the top-level.
 */
void PERSProblem::Domain::leaveContext(void) {
	ASSERT(!isBottom);
	Item *ptr = data.pop();
	delete ptr;
}


static SilentFeature::Maker<ACSBuilder> maker;
/**
 * Feature ensuring that the persistence analysis has been performed.
 *
 * @p Configuration
 * @li @ref DATA_PSEUDO_UNROLLING
 * @li @ref DATA_FIRSTMISS_LEVEL
 * @li @ref ENTRY_PERS_ACS
 *
 * @p Properties
 * @li @ref PERS_ACS
 * @li @ref LEVEL_PERS_ACS
 * @ingroup dcache
 */
SilentFeature PERS_ACS_FEATURE("otawa::dcache::PERS_ACS_FEATURE", maker);


/**
 * This identifier gives an array of ACS for the persistence analysis.
 * The array contains as many ACS as set in the analyzed cache, one
 * for each set. The blocks in the ACS have an age between -1 (not loaded in the cache)
 * and the associativity of the cache (loaded but also wiped out).
 *
 * @p Feature
 * @li @ref PERS_ACS_FEATURE
 *
 * @p Hooks
 * @li @ref BasicBlock
 *
 * @ingroup dcache
 */
Identifier<genstruct::Vector<ACS *> *> PERS_ACS("otawa::dcache::PERS_ACS", 0);


/**
 * This configuration property allows to provide an non-empty ACS
 * for the persistence analysis.
 *
 * @p Feature
 * @li @ref PERS_ACS_FEATURE
 *
 * @ingroup dcache
 */
Identifier<genstruct::Vector<ACS *> *> ENTRY_PERS_ACS("otawa::dcache::ENTRY_PERS_ACS", 0);


/**
 * This identifier allows to get an array on a stack of ACS,
 * one entry for each set of the analyzed cache. Each stack item in the stack
 * represent the persistent analysis ACS of the loops embedding the basic block this
 * property is hooked to.
 *
 * @ingroup dcache
 */
Identifier<acs_stack_table_t *> LEVEL_PERS_ACS("otawa::dcache::LEVEL_PERS_ACS", 0);

} }	// otawa::dcache
