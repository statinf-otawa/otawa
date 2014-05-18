/*
 *	otawa::dcache::DIRTY_FEATURE
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/dcache/features.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/util/HalfAbsInt.h>

namespace otawa { namespace dcache {

/*
 * ABSTRACT DOMAIN
 * 		S: B -> m: B x M: B where
 * 			m: may be dirty
 * 			M: must be dirty
 * 		whose values means:
 * 			0, 0	not dirty at all
 * 			0, 1	impossible (bottom, see below for demonstration)
 * 			1, 0	may be dirty (T)
 * 			1, 1	must be dirty
 * 		the join function is:
 * 		J: m1: B x M1: B x m2: B x M2: B -> m: B x M: B s.t.
 * 			m = m1 \/ m2 /\ M = M1 /\ M2
 * 		therefore J(m, M, 0, 1) = m \/ 0 = m, M /\ 1 = M
 *
 * 		The order is:
 * 			1, 0 > 0, 0
 * 			1, 0 > 1, 1
 * 			0, 0 > 0, 1
 * 			1, 1 > 0, 1
 */

template <class P>
class CacheListener {
public:
	typedef P Problem;
	CacheListener(WorkSpace *ws, Problem& prob, int set, Identifier<typename P::Domain *>& id)
	: _prob(prob), _set(set), _id(id) {
		const CFGCollection *col = INVOLVED_CFGS(ws);
		for(int i = 0; i < col->count();  i++)
			for (CFG::BBIterator bb(col->get(i)); bb; bb++)
				prob.init(_id(bb)[_set], prob.bottom());
	}

	inline Problem& getProb(void) const { return _prob; }

private:
	Problem& _prob;
	int _set;
	Identifier<typename P::Domain *>& _id;
};


/**
 * @class DirtyManager
 * A dirty manager allows to exploit the results of the dirty analysis (@ref DIRTY_FEATURE).
 * The code below shows how to use it:
 *
 * @code
 * DirtyManager man(my_block_row, my_cache);
 * DirtyManager::t s = man.clone(DIRTY(my_bb));
 * Pair<int, BlockAccess *> accs = DATA_BLOCKS(my_bb);
 * for(int i = 0; i < accs.fst; i++) {
 * 	man.update(s, accs.snd[i]);
 * 	if(man.mustBeDirty(d, my_cache_block))	...;
 * 	if(man.mayBeDirty(d, my_cache_block)) ...;
 * }
 * @endcode
 */


/**
 * @typedef DirtyManager::t;
 * State for the dirty analysis.
 */


DirtyManager::DirtyManager(const BlockCollection& coll)
: _coll(coll), bot(coll.count()), _top(coll.count()) {
	bot.may().empty();
	bot.must().fill();
	_top.may().fill();
	_top.must().empty();
}


/**
 * Test if the given block may be dirty.
 * @param state		Dirty state.
 * @param block		Block to test.
 * @return			True if it may be dirty, false else.
 */
bool DirtyManager::mayBeDirty(const t& state, int block) const {
	return state.may().contains(block);
}

/**
 * Test if the given block must be dirty.
 * @param state		Dirty state.
 * @param block		Block to test.
 * @return			True if it must be dirty, false else.
 */
bool DirtyManager::mustBeDirty(const t& state, int block) const {
	return state.must().contains(block);
}


/**
 * Build a bottom dirty state.
 */
const DirtyManager::t& DirtyManager::bottom(void) const {
	return bot;
}


/**
 * Build a top dirty state.
 */
const DirtyManager::t& DirtyManager::top(void) const {
	return _top;
}


/**
 * Copy a state in an existing one.
 * @param d		State to copy to.
 * @param s		State to copy from.
 */
void DirtyManager::set(t& d, const t& s) const {
	d.may() = s.may();
	d.must() = s.must();
}


/**
 * Test if both dirty states are equals.
 * @param s1	First state to compare.
 * @param s2	Second state to compare.
 * @return		True if they are equal, false else.
 */
bool DirtyManager::equals(const t& s1, const t& s2) const {
	return s1.must().equals(s2.must()) && s1.may().equals(s2.may());
}


/**
 * Update the dirty state with the given block access.
 * @param d		Dirty state to change (input and output of the update).
 * @param acc	Block access to update with.
 */
void DirtyManager::update(t& d, const BlockAccess& acc) {
	switch(acc.action()) {
	case BlockAccess::LOAD:
		break;

	case BlockAccess::STORE:
		switch(acc.kind()) {
		case BlockAccess::BLOCK:
			if(acc.block().set() == _coll.cacheSet()) {
				d.may().add(acc.block().index());
				d.must().add(acc.block().index());
			}
			break;
		case BlockAccess::RANGE: {
				int first = -1;
				bool many = false;
				for(int i = 0; i < _coll.count(); i++)
					if(acc.inRange(_coll[i].address().offset())) {
						if(first < 0)
							first = i;
						else {
							d.may().add(i);
							many = true;
						}
					}
				if(first >= 0) {		// more precise result with only one block
					d.may().add(first);
					if(!many)
						d.must().add(first);
				}
			}
			break;
		case BlockAccess::ANY:
			d.may().fill();
			break;
		}
		break;

	case BlockAccess::PURGE:
		switch(acc.kind()) {
		case BlockAccess::BLOCK:
			if(acc.block().set() == _coll.cacheSet()) {
				d.may().remove(acc.block().index());
				d.must().remove(acc.block().index());
			}
			break;
		case BlockAccess::RANGE: {
				for(int i = 0; i < _coll.count(); i++) {
					int first = -1;
					bool many = false;
					if(acc.inRange(_coll[i].address().offset())) {
						if(first <  0)
							first = i;
						else {
							d.must().remove(i);
							many = true;
						}
					}
					if(first >= 0) {		// more precise result with only one block
						d.must().remove(first);
						if(!many)
							d.may().remove(first);
					}
				}
			}
			break;
		case BlockAccess::ANY:
			d.must().empty();
			break;
		}
		break;
	}
}


/**
 * Join two dirty states.
 * @param d		First and target dirty state.
 * @param s		Second dirty state.
 */
void DirtyManager::join(t& d, const t& s) const {
	//cerr << "J(" << d << ", " << s << ") = ";
	d.may() |= s.may();
	d.must() &= s.must();
	//cerr << d << io::endl;
}


void DirtyManager::t::print(io::Output& out) const {
	out << "{ ";
	bool first = true;
	for(int i = 0; i < _may.size(); i++) {
		if(first)
			first = false;
		else
			out << ", ";
		if(_may.contains(i)) {
			if(_must.contains(i))
				out << 'M';
			else
				out << 'm';
		}
		else {
			if(_must.contains(i))
				out << '_';
			else
				out << '-';
		}
	}
	out << " }";
}



// Problem
class Problem {
public:
	typedef DirtyManager::t Domain;
	Problem(const BlockCollection& coll): man(coll) { }
	inline const Domain& bottom(void) const { return man.bottom(); }
	inline const Domain& entry(void) const { return man.bottom(); }	/* very ugly and unsound, should be fixed */
	inline void lub(Domain &a, const Domain &b) const { man.join(a, b); }
	inline void assign(Domain &a, const Domain &b) const { man.set(a, b); }
	inline bool equals(const Domain &a, const Domain &b) const { return man.equals(a, b); }
	inline void init(Domain& d, const Domain& s) const { man.set(d, s); }
	inline void update(Domain& out, const Domain& in, BasicBlock* bb) {
		Pair<int, BlockAccess *> blocks = DATA_BLOCKS(bb);
		man.set(out, in);
		for(int i = 0; i < blocks.fst; i++)
			man.update(out, blocks.snd[i]);
	}
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
private:
	DirtyManager man;
};



/**
 * Default processor to perform dirty analysis. Dirty analysis is used with data caches using
 * a write-back policy, that is, when a block is modified, the modification is only kept in the cache
 * until the block is wiped out. In this case, if the block is dirty, it must be first saved to memory
 * before loading a new one.
 *
 * @p Required Features
 * @li @ref DATA_BLOCK_FEATURE
 * @li @ ref COLLECTED_CFG_FEATURE
 * @li @ref hard::CACHE_CONFIGURATION_FEATURE
 *
 * @p Provided Features
 * @li @ref DIRTY_FEATURE
 */
class DirtyAnalysis: public Processor {
public:
	static p::declare reg;
	DirtyAnalysis(p::declare& r = reg): Processor(r) { }

protected:
	virtual void processWorkSpace(WorkSpace *ws) {

		// get information
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION(ws);
		const hard::Cache *cache = conf->dataCache();
		if(!cache)
			throw ProcessorException(*this, "no data cache !");
		const BlockCollection *colls = DATA_BLOCK_COLLECTION(ws);

		// initialize the dirty sets
		for(CFGCollection::Iterator cfg(otawa::INVOLVED_CFGS(ws)); cfg; cfg++)
			for(CFG::BBIterator bb(cfg); bb; bb++)
				DIRTY(bb).ref().allocate(cache->rowCount());

		// perform the analysis
		for(int i = 0; i < cache->rowCount(); i++)
			if(colls[i].count())
				processColl(ws, colls[i]);
	}

	void processColl(WorkSpace *ws, const BlockCollection& coll) {

		// resolve the problem
		Problem prob(coll);
		DefaultListener<Problem> listener(ws, prob);
		DefaultFixPoint<DefaultListener<Problem> > fp(listener);
		util::HalfAbsInt<DefaultFixPoint<DefaultListener<Problem> > > hai(fp, *ws);
		hai.solve();

		// put the results
		for(CFGCollection::Iterator cfg(otawa::INVOLVED_CFGS(ws)); cfg; cfg++)
			for(CFG::BBIterator bb(cfg); bb; bb++)
				(*DIRTY(bb))[coll.cacheSet()] = *listener.results[cfg->number()][bb->number()];
	}
};

p::declare DirtyAnalysis::reg = p::init("otawa::dcache::DirtyAnalysis", Version(1, 0, 0))
	.require(DATA_BLOCK_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.provide(DIRTY_FEATURE)
	.maker<DirtyAnalysis>();


static SilentFeature::Maker<DirtyAnalysis> maker;
/**
 * This feature is only useful for data cache with write-back mechanism.
 * In this case, it is needed before wiping out a block if it dirty (modified) or not.
 * If it is dirty, additional time is spent to writing its value back to memory.
 * Notice that the dirty state is made of a MAY component (better case dirty)
 * and a MUST component (worst case dirty). If the block is in the MUST set, it is asserted
 * it is dirty. If the block is not in the MAY set, it is asserted it is not dirty. Else
 * it must be dirty or not (not-classified case).
 *
 * @p Properties
 * @li @ref DIRTY
 *
 * @p Default Processor
 * @li @ref DirtyAnalysis
 */
SilentFeature DIRTY_FEATURE("otawa::dcache::DIRTY_FEATURE", maker);


/**
 * This property contains information about the dirty state of a block.
 * To use it, creates a @ref DirtyManager with the target row of the data cache,
 * the DIRTY value to methods mayBeDirty() or mustBeDirty() and reply the sequence
 * of @ref BlockAccess of the concerned basic block.
 *
 * @p Hook
 * @li @ref otawa::BasicBlock
 */
Identifier<AllocatedTable<DirtyManager::t> > DIRTY("otawa::dcache::DIRTY", AllocatedTable<DirtyManager::t>::EMPTY);

} }		// otawa::dcache

