/*
 *	$Id$
 *	SubCFGBuilder class implementation
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
#include <elm/util/BitVector.h>
#include <elm/genstruct/VectorQueue.h>
#include <elm/genstruct/HashTable.h>
#include <elm/Iterator.h>

using namespace elm;

DEFINE_PROC(otawa::SubCFGBuilder,
	version(1, 0, 0);
	require(COLLECTED_CFG_FEATURE);
	use(VIRTUALIZED_CFG_FEATURE);
	invalidate(COLLECTED_CFG_FEATURE);
	provide(VIRTUALIZED_CFG_FEATURE);
)

namespace otawa {

Identifier<bool> IS_START("", false);
Identifier<bool> IS_STOP("", false);

#ifdef OTAWA_AIL_DEBUG
#	define OTAWA_AILD(x)	x
#else
#	define OTAWA_AILD(x)
#endif

template <class G, class T>
class AbsIntLite {
public:

	inline AbsIntLite(const G& graph, const T& domain): g(graph), d(domain) {
		vals = new typename T::t[g.count()];
		for(int i = 0; i < g.count(); i++)
			d.set(vals[i], d.bottom());
	}

	inline void process(void) {

		// initialization
		genstruct::VectorQueue<typename G::Vertex> todo;
		d.set(vals[g.index(g.entry())], d.initial());
		for(typename G::Successor succ(g, g.entry()); succ; succ++)
			todo.put(succ);

		// loop until fixpoint
		while(todo) {
			typename G::Vertex v = todo.get();
			OTAWA_AILD(cerr << "NEXT: " << v << io::endl);

			// join of predecessor
			d.set(tmp, d.bottom());
			for(typename G::Predecessor pred(g, v); pred; pred++)
				d.join(tmp, vals[g.index(pred)]);
			OTAWA_AILD(cerr << "- JOIN IN: "; d.dump(cerr, tmp); cerr << io::endl);

			// update value
			d.update(v, tmp);
			OTAWA_AILD(cerr << "- UPDATE: "; d.dump(cerr, tmp); cerr << io::endl);

			// new value ?
			if(d.equals(d.bottom(), vals[g.index(v)]) || !d.equals(tmp, vals[g.index(v)])) {
				OTAWA_AILD(cerr << "- NEW VALUE\n");
				d.set(vals[g.index(v)], tmp);

				// add successors
				for(typename G::Successor succ(g, v); succ; succ++) {
					todo.put(*succ);
					OTAWA_AILD(cerr << "- PUTTING " << *succ << io::endl);
				}
			}
		}
	}

	inline const typename T::t& in(typename G::Vertex v) {
		d.set(tmp, d.bottom());
		for(typename G::Predecessor pred(g, v); pred; pred++)
			d.join(tmp, vals[g.index(pred)]);
		return tmp;
	}

	inline const typename T::t& out(typename G::Vertex v) const {
		return vals[g.index(v)];
	}

private:
	const G& g;
	T d;
	typename T::t *vals;
	typename T::t tmp;
};

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


class ForwardCFGAdapter {
public:

	typedef BasicBlock *Vertex;

	inline ForwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->countBB(); }
	inline Vertex entry(void) const { return cfg->entry(); }
	inline int index(Vertex v) const { return v->number(); }

	class Predecessor: public PreIterator<Predecessor, Vertex> {
	public:
		inline Predecessor(const ForwardCFGAdapter& g, const Vertex& v): iter(v) { }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->source(); }
		void next(void) { iter++; }
	private:
		BasicBlock::InIterator iter;
	};

	class Successor: public PreIterator<Successor, Vertex> {
	public:
		inline Successor(const ForwardCFGAdapter& g, const Vertex& v): iter(v) { step(); }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->target(); }
		void next(void) { iter++; step(); }
	private:
		inline void step(void) {
			while(iter && iter->kind() == Edge::CALL)
				iter++;
		}
		BasicBlock::OutIterator iter;
	};

private:
	CFG *cfg;
};


class BackwardCFGAdapter {
public:

	typedef BasicBlock *Vertex;

	inline BackwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->countBB(); }
	inline Vertex entry(void) const { return cfg->exit(); }
	inline int index(Vertex v) const { return v->number(); }

	class Successor: public PreIterator<Successor, Vertex> {
	public:
		inline Successor(const BackwardCFGAdapter& g, const Vertex& v): iter(v) { }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->source(); }
		void next(void) { iter++; }
	private:
		BasicBlock::InIterator iter;
	};

	class Predecessor: public PreIterator<Predecessor, Vertex> {
	public:
		inline Predecessor(const BackwardCFGAdapter& g, const Vertex& v): iter(v) { step(); }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->target(); }
		void next(void) { iter++; step(); }
	private:
		inline void step(void) {
			while(iter && iter->kind() == Edge::CALL)
				iter++;
		}
		BasicBlock::OutIterator iter;
	};

private:
	CFG *cfg;
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
	CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);

	// fix start and stops
	if(!start) {
		BasicBlock::OutIterator next(cfg->entry());
		ASSERT(start);
		start = next->target()->address();
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
	cout << "\nFORWARD\n";
	for(CFG::BBIterator bb(cfg); bb; bb++)
		cout << *bb
			 << "\tIN=" << Domain::toString(start_ai.in(bb))
			 << "\tOUT=" << Domain::toString(start_ai.out(bb))
			 << io::endl;

	// stop flood analysis
	StopDomain stop_dom(start, stops);
	BackwardCFGAdapter stop_adapter(cfg);
	AbsIntLite<BackwardCFGAdapter, StopDomain> stop_ai(stop_adapter, stop_dom);
	stop_ai.process();
	cout << "\nBACKWARD\n";
	for(CFG::BBIterator bb(cfg); bb; bb++)
		cout << *bb
			 << "\tIN=" << Domain::toString(stop_ai.in(bb))
			 << "\tOUT=" << Domain::toString(stop_ai.out(bb))
			 << io::endl;

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
	}
	vcfg->addBB(vcfg->exit());

	// build the virtual edges
	for(genstruct::HashTable<BasicBlock *, BasicBlock *>::PairIterator pair(bbs); pair; pair++) {
		BasicBlock *src = (*pair).fst, *vsrc = (*pair).snd;

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
			if(edge->kind() != Edge::CALL) {
				BasicBlock *vtarget = bbs.get(edge->target());
				if(vtarget) {
					new Edge(vsrc, vtarget, edge->kind());
					// !!TODO!! handle virtual properties here
				}
			}
			else
				new Edge(vsrc, edge->target(), edge->kind());
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

