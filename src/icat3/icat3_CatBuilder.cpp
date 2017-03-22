/*
 *	icat3::CatBuilder class implementation
 *	Copyright (c) 2017, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <otawa/cfg.h>
#include <otawa/hard/Memory.h>
#include <otawa/icache/features.h>
#include <otawa/icat3/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/program.h>

#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

class ACSManager {
public:
	typedef MustPersDomain::t acs_t;

	ACSManager(const LBlockCollection& c): coll(c), _b(0) {
		mustpers = new MustPersDomain *[coll.sets()];
		acss = new acs_t[coll.sets()];
		for(int i = 0; i < coll.sets(); i++)
			mustpers[i] = new MustPersDomain(coll, i);
	}

	~ACSManager(void) {
		for(int i = 0; i < coll.sets(); i++)
			delete mustpers[i];
		delete [] mustpers;
		delete [] acss;
	}

	void start(Block *b) {
		ASSERT(b);
		_b = b;
		used.clear();
	}

	acs_t& get(const LBlock *lb) {
		ASSERT(lb);
		if(!used.contains(lb->set())) {
			used.add(lb->set());
			acss[lb->set()] = acs_t((*MUST_IN(_b))[lb->set()], (*PERS_IN(_b))[lb->set()]);
			cerr << "DEBUG: getting acs for " << lb << ": "; mustpers[lb->set()]->print(acss[lb->set()], cerr); cerr << io::endl;
		}
		return acss[lb->set()];
	}

	void update(const icache::Access& acc) {
		LBlock *lb = LBLOCK(acc);
		mustpers[lb->set()]->update(acc, get(lb));
	}

	int mustAge(const LBlock *lb) {
		return mustpers[lb->set()]->must(get(lb))[lb->index()];
	}

	int depth(const LBlock *lb) {
		return mustpers[lb->set()]->pers(get(lb)).depth();
	}

	int persAge(const LBlock *lb, int depth) {
		return mustpers[lb->set()]->pers(get(lb))[depth][lb->index()];
	}


private:
	const LBlockCollection& coll;
	MustPersDomain **mustpers;
	acs_t *acss;
	Vector<int> used;
	Block *_b;
};

/**
 */
class CatBuilder: public BBProcessor {
public:
	static p::declare reg;
	CatBuilder(void): BBProcessor(reg), coll(0), mem(0), A(0), man(0)  { }

protected:

	typedef MustPersDomain::t acs_t;

	/**
	 */
	virtual void setup(WorkSpace *ws) {
		coll = icat3::LBLOCKS(ws);
		ASSERT(coll);
		A = coll->A();
		mem = hard::MEMORY(ws);
		ASSERT(mem);
		man = new ACSManager(*coll);
	}

	/**
	 */
	virtual void cleanup(WorkSpace *ws) {
		delete man;
	}

	/**
	 */
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *v) {
		for(Block::EdgeIter e = v->ins(); e; e++) {
			man->start(v);
			processAccesses(v, icache::ACCESSES(v));
			processAccesses(v, icache::ACCESSES(e));
		}
	}

	/**
	 * Generate and store the events.
	 * @param tu	Time unit to put the event in.
	 * @param place	Place of the contribution.
	 * @param cv	Counting vertex.
	 * @param iv	Input vertex.
	 * @param cont	Contribution.
	 */
	void processAccesses(Block *v, Bag<icache::Access>& accs) {
		for(int i = 0; i < accs.count(); i++) {
			cerr << "DEBUG: " << accs[i] << io::endl;
			LBlock *lb = LBLOCK(accs[i]);
			age_t age = man->mustAge(lb);
			if(0 <= age && age < A)
				CATEGORY(accs[i]) = AH;
			else {
				LoopIter h(v);
				for(int d = man->depth(lb) - 1; d >= 0 && h; d--, h++) {
					age = man->persAge(lb, d);
					if(0 <= age && age < A) {
						CATEGORY(accs[i]) = NC;
						HEADER(accs[i]) = h;
						break;
					}
				}
				CATEGORY(accs[i]) = NC;
			}
			if(logFor(LOG_BLOCK)) {
				log << "\t\t\t\t" << accs[i] << ": " << CATEGORY(accs[i]);
				if(CATEGORY(accs[i]) == PE)
					log << " (" << HEADER(accs[i]) << ")";
				log << io::endl;
			}
		}
	}

	const LBlockCollection *coll;
	const hard::Memory *mem;
	int A;
	ACSManager *man;
};

p::declare CatBuilder::reg = p::init("otawa::icat3::CatBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.make<CatBuilder>()
	.provide(CATEGORY_FEATURE)
	.require(LBLOCKS_FEATURE)
	.require(MUST_PERS_ANALYSIS_FEATURE)
	.require(hard::MEMORY_FEATURE);


/**
 * This feature ensures that category information is stored on each @ref icache::Access found
 * in the blocks and in the CFG of the current workspace. Such information is made of
 * @ref CATEGORY specifying the behaviour of the cache (AH -- Always Hit, AM -- Always Miss,
 * PE -- Persistent and NC -- Not-classified) and, in the PE case, the loop header causing
 * the considered block to be evicted.
 *
 * @par Properties
 * @li @ref CATEGORY
 * @li @ref HEADER
 *
 * @par Default implementation
 * @li @ref CatBuilder
 *
 * @ingroup icat3
 */
p::feature CATEGORY_FEATURE("otawa::icat3::CATEGORY_FEATURE", CatBuilder::reg);


/**
 * Defines the instruction cache category for the @ref icache::Access where
 * this property is set.
 *
 * @par Hooks
 * @li @ref icache::Access
 *
 * @par Features
 * @li @ref CATEGORY_FEATURE
 *
 * @ingroup icat3
 */
p::id<category_t> CATEGORY("otawa::icat3::CATEGORY", UC);


/**
 * In the case of a category of type PE (persistent), this property
 * provides the loop header that causes the considered block to be
 * eviected.
 *
 * @par Hooks
 * @li @ref icache::Access
 *
 * @par Features
 * @li @ref CATEGORY_FEATURE
 *
 * @ingroup icat3
 */
p::id<Block *> HEADER("otawa::icat3::HEADER", 0);

} }	// otawa::icat3






