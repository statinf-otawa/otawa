/*
 *	$Id$
 *	SubCFGBuilder analyzer implementation
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

#include <otawa/cfg/SubCFGBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>
#include <otawa/dfa/AbsIntLite.h>
#include <otawa/cfg/CFGAdapter.h>
#include <elm/util/BitVector.h>
#include <elm/genstruct/HashTable.h>

using namespace elm;
using namespace otawa::dfa;

namespace otawa {

/**
 */
SubCFGBuilder::SubCFGBuilder(): Processor(reg) {
}

Registration<SubCFGBuilder> SubCFGBuilder::reg(
	"otawa::SubCFGBuilder",
	Version(1, 0, 0),
	p::require, &COLLECTED_CFG_FEATURE,
	p::use, &VIRTUALIZED_CFG_FEATURE,
	p::invalidate, &COLLECTED_CFG_FEATURE,
	p::provide, &VIRTUALIZED_CFG_FEATURE,
	p::end
);


Identifier<bool> IS_START("", false);
Identifier<bool> IS_STOP("", false);

class Domain {
public:
	typedef char t;
	static const char BOTTOM = -1,
					  FALSE = 0,
					  TRUE = 1;
	inline t initial(void) const { return FALSE; }
	inline t bottom(void) const { return BOTTOM; }
	inline void join(t& d, t s) const { if(d < s) d = s; }
	inline bool equals(t v1, t v2) const { return v1 == v2; }
	inline void set(t& d, t s) const { d = s; }

	inline static char toString(t c) {
		switch(c) {
		case BOTTOM: return '_';
		case FALSE: return 'F';
		case TRUE: return 'T';
		default: return '?';
		}
	}

	inline void dump(io::Output& out, t c) { out << toString(c); }
};

class StartDomain: public Domain {
public:
	inline StartDomain(const Address& _start, const genstruct::Vector<Address>& _stops)
	: start(_start), stops(_stops) { }

	void update(BasicBlock *bb, t& d) {
		if(bb->isEnd())
			return;
		if(IS_START(bb))
			d = TRUE;
		if(IS_STOP(bb))
			d = FALSE;
	}

private:
	const Address& start;
	const genstruct::Vector<Address> &stops;
};


class StopDomain: public Domain {
public:
	inline StopDomain(const Address& _start, const genstruct::Vector<Address>& _stops)
	: start(_start), stops(_stops) { }

	void update(BasicBlock *bb, t& d) {
		if(bb->isEnd())
			return;
		if(IS_STOP(bb))
			d = TRUE;
		if(IS_START(bb))
			d = FALSE;
	}

private:
	const Address& start;
	const genstruct::Vector<Address> &stops;
};


/**
 * @class SubCFGBuilder
 * Build a sub-CFG starting at the given @ref otawa::START address and
 * ending at the multiple @ref otawa::STOP addresses.
 *
 * Note that, in this version, START and STOP addresses must be part
 * of the same CFG (original CFG or virtualized CFG).
 *
 * @par Required features
 * @li @ref otawa::VIRTUALIZED_CFG_FEATURE
 *
 * @par Provided features
 * @li @ref otawa::VIRTUALIZED_CFG_FEATURE
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 *
 * @par Configuration
 * @li @ref otawa::CFG_START
 * @li @ref otawa::CFG_STOP
 */


/**
 */
void SubCFGBuilder::configure(const PropList &props) {
	Processor::configure(props);
	start = CFG_START(props);
	stops.clear();
	for(Identifier<Address>::Getter stop(props, CFG_STOP); stop; stop++)
		stops.add(stop);
}


/**
 */
void SubCFGBuilder::processWorkSpace(WorkSpace *ws) {

	// get the CFG
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);

	// fix start and stops
	if(!start) {
		BasicBlock::OutIterator next(cfg->entry());
		start = next->target()->address();
		ASSERT(start);
	}
	if(!stops)
		for(BasicBlock::InIterator prev(cfg->exit()); prev; prev++)
			stops.add(prev->source()->address());

	// mark nodes
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		if(bb->isEnd())
			continue;
		if(bb->address() <= start && start < bb->address() + bb->size())
			IS_START(bb) = true;
		for(genstruct::Vector<Address>::Iterator stop(stops); stop; stop++)
			if(bb->address() <= *stop && *stop < bb->address() + bb->size()) {
				IS_STOP(bb) = true;
				break;
			}
	}

	// start flood analysis
	StartDomain start_dom(start, stops);
	ForwardCFGAdapter start_adapter(cfg);
	AbsIntLite<ForwardCFGAdapter, StartDomain> start_ai(start_adapter, start_dom);
	start_ai.process();
	/*cout << "\nFORWARD\n";
	for(CFG::BBIterator bb(cfg); bb; bb++)
		cout << *bb
			 << "\tIN=" << Domain::toString(start_ai.in(bb))
			 << "\tOUT=" << Domain::toString(start_ai.out(bb))
			 << io::endl;*/

	// stop flood analysis
	StopDomain stop_dom(start, stops);
	BackwardCFGAdapter stop_adapter(cfg);
	AbsIntLite<BackwardCFGAdapter, StopDomain> stop_ai(stop_adapter, stop_dom);
	stop_ai.process();
	/*cout << "\nBACKWARD\n";
	for(CFG::BBIterator bb(cfg); bb; bb++)
		cout << *bb
			 << "\tIN=" << Domain::toString(stop_ai.in(bb))
			 << "\tOUT=" << Domain::toString(stop_ai.out(bb))
			 << io::endl;*/

	// find list of accepted nodes
	cout << "\nRESULT\n";
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		if(IS_START(bb)
		|| IS_STOP(bb)
		|| (start_ai.in(bb) == Domain::TRUE && stop_ai.in(bb) == Domain::TRUE))
			cout << " - " << *bb << io::endl;
	}

	// build the new CFG
	vcfg = new VirtualCFG(false);
	genstruct::HashTable<BasicBlock *, BasicBlock *> bbs;
	genstruct::Vector<BasicBlock *> orgs;

	// make all virtual BB
	vcfg->addBB(vcfg->entry());
	for(CFG::BBIterator bb(cfg); bb; bb++) {

		// remove non-useful blocks
		if(bb->isEnd())
			continue;
		if(!IS_START(bb)
		&& !IS_STOP(bb)
		&& (start_ai.in(bb) != Domain::TRUE || stop_ai.in(bb) != Domain::TRUE))
			continue;

		// build the new basic block
		BasicBlock *vbb = new VirtualBasicBlock(bb);
		vcfg->addBB(vbb);
		bbs.put(bb, vbb);
		orgs.add(bb);
	}
	vcfg->addBB(vcfg->exit());

	// build the virtual edges
	for(genstruct::Vector<BasicBlock *>::Iterator src(orgs); src; src++) {
		BasicBlock *vsrc = bbs.get(src, 0);
		ASSERT(vsrc);

		// manage start
		if(IS_START(src)) {
			new Edge(vcfg->entry(), vsrc, Edge::VIRTUAL_CALL);
			src->removeProp(IS_START);
		}

		// manage stop
		if(IS_STOP(src)) {
			new Edge(vsrc, vcfg->exit(), Edge::VIRTUAL_RETURN);
			src->removeProp(IS_STOP);
			continue;
		}

		// manage successors
		for(BasicBlock::OutIterator edge(src); edge; edge++) {

			// !!BUGGY!! we should build new CFG for the collection !
			if(edge->kind() == Edge::CALL)
				new Edge(vsrc, edge->target(), edge->kind());

			// try to duplicate
			else {
				BasicBlock *vtarget = bbs.get(edge->target(), 0);
				if(vtarget) {
					new Edge(vsrc, vtarget, edge->kind());

					// take care of VIRTUAL_RETURN_BLOCK
					if(edge->kind() == Edge::VIRTUAL_CALL) {
						BasicBlock *ret = VIRTUAL_RETURN_BLOCK(src);
						BasicBlock *vret = bbs.get(ret);
						VIRTUAL_RETURN_BLOCK(vsrc) = vret ? vret :  vcfg->exit();
					}
				}
			}
		}
	}

	// finalize the new CFG
	vcfg->numberBBs();
}


void SubCFGBuilder::cleanup (WorkSpace *ws) {
	track(VIRTUALIZED_CFG_FEATURE, ENTRY_CFG(ws) = vcfg);
}


/**
 * Configuration of @ref otawa::SubCFGBuilder specifying the start of the sub-CFG
 * to process. If not provided, the start is assumed to be the ENTRY of the CFG.
 */
Identifier<Address> CFG_START("otawa::CFG_START");


/**
 * Configuration of @ref otawa::SubCFGBuilder specifying a end of the sub-CFG
 * to process. Several properties of this type are accepted.
 * If not provided, the stop is assumed to be the EXIT of the original CFG.
 */
Identifier<Address> CFG_STOP("otawa::CFG_STOP");

}	// otawa

