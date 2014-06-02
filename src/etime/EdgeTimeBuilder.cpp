/*
 *	EdgeTimeBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <otawa/etime/EdgeTimeBuilder.h>
#include <elm/avl/Set.h>
#include <otawa/etime/features.h>

namespace otawa { namespace etime {

class EventComparator {
public:
	static int compare (const Event *e1, const Event *e2)
		{ return e1->inst()->address().compare(e2->inst()->address()); }
};


/**
 * Convert an event mask to a string.
 * @param mask		Event mask.
 * @param len		Number of events.
 * @return			String representing the event mask.
 */
string maskToString(t::uint32 mask, int len) {
	StringBuffer buf;
	for(int i = 0; i < len; i++)
		buf << ((mask & (1 << i)) ? "!" : "_");
	return buf.toString();
}


/**
 * Represents a configuration for computing cost of a BB.
 * Each bits represents the state of an event:
 * @li 0	event not activated
 * @li 1	event activated
 */
class Config {
public:
	inline Config(void): b(0) { }
	inline Config(t::uint32 bits): b(bits) { }
	inline Config(const Config& conf): b(conf.b) { }
	inline t::uint32 bits(void) const { return b; }
	inline void set(int n) { b |= 1 << n; }
	inline void clear(int n) { b &= ~(1 << n); }
	inline bool bit(int n) { return b & (1 << n); }

	string toString(int n) {
		StringBuffer buf;
		for(int i = 0; i < n; i++)
			buf << (bit(i) ? "+" : "-");
		return buf.toString();
	}

private:
	t::uint32 b;
};


/**
 * Set of configurations with the same or with a maximum of time.
 */
class ConfigSet {
public:
	ConfigSet(void): t(0) { }
	ConfigSet(ot::time time): t(time) { }
	ConfigSet(const ConfigSet& set): t(set.t), confs(set.confs) { }
	inline ot::time time(void) const { return t; }
	inline int count(void) const { return confs.length(); }
	inline void add(Config conf) { confs.add(conf); }
	inline void add(ConfigSet& set) { t = max(t, set.time()); confs.addAll(set.confs); }

	t::uint32 posConst(void)  {
		t::uint32 r = 0xffffffff;
		for(int i = 0; i < confs.length(); i++)
			r &= confs[i].bits();
		return r;
	}

	t::uint32 negConst(void) {
		t::uint32 r = 0;
		for(int i = 0; i < confs.length(); i++)
			r |= confs[i].bits();
		return ~r;
	}

	t::uint32 unused(t::uint32 neg, t::uint32 pos, int n) {
		t::uint32 mask = neg | pos;
		t::uint32 r = 0;
		for(int i = 0; i < n; i++)
			if(!(mask & (1 << i))) {
				genstruct::Vector<t::uint32> nconf, pconf;
				for(int j = 0; j < confs.length(); j++) {
					t::uint32 c = confs[j].bits();
					if(c & (1 << i)) {
						c &= ~(1 << i);
						if(nconf.contains(c))
							nconf.remove(c);
						else
							pconf.add(c);
					}
					else {
						if(pconf.contains(c))
							pconf.remove(c);
						else
							nconf.add(c);
					}
				}
				if(!pconf && !nconf)
					r |= 1 << i;
			}
		return r;
	}

	t::uint32 complex(t::uint32 neg, t::uint32 pos, t::uint32 unused, int n) {
		return ((1 << n) - 1) & ~neg & ~pos & ~unused;
	}

	class Iter: public genstruct::Vector<Config>::Iterator {
	public:
		inline Iter(const ConfigSet& set): genstruct::Vector<Config>::Iterator(set.confs) { }
	};

private:
	ot::time t;
	genstruct::Vector<Config> confs;
};


/**
 * Collects variables linking events with blocks in ILP.
 */
class EventCollector {
public:

	EventCollector(Event *event): evt(event), imprec(0) { }

	/**
	 * Case for the variable between place (prefix/block) and activation of the event (on/off).
	 */
	typedef enum {
		PREFIX_OFF = 0,
		PREFIX_ON = 1,
		BLOCK_OFF = 2,
		BLOCK_ON = 3,
		SIZE = 4
	} case_t;

	inline Event *event(void) const { return evt; }

	/**
	 * Add a variable contribution.
	 * @param c		Case of contribution.
	 * @param var	Variable of contribution (null for blurred contribution driving to estimation).
	 */
	void contribute(case_t c, ilp::Var *var) {
		if(!var)
			imprec |= 1 << c;
		else
			vars[c].add(var);
	}

	/**
	 * Make the variable and constraint for the current event.
	 * According to the variable list and the provided overestimation,
	 * may generate zero, one or several constraints.
	 * @param sys	System to create constraints in.
	 */
	void make(ilp::System *sys) {
		for(int c = PREFIX_ON; c < SIZE; ++c)
			if(vars[c] && evt->isOverestimating(isOn(case_t(c)))) {
				ilp::Constraint *cons = sys->newConstraint(
					"event constraint",
					(imprec & (1 << c)) ? ilp::Constraint::GE : ilp::Constraint::EQ);
				evt->overestimate(cons, isOn(case_t(c)));
				for(genstruct::SLList<ilp::Var *>::Iterator v(vars[c]); v; v++)
					cons->addRight(1, *v);
			}
	}

	/**
	 * Test if the event is on or off.
	 * @param c		Case to test.
	 * @return		True if on, false if off.
	 */
	inline bool isOn(case_t c) {
		static bool ons[SIZE] = { false, true, false, true };
		return ons[c];
	}

	/**
	 * Build an event selector case.
	 */
	static case_t make(EdgeTimeBuilder::place_t place, bool on) {
		switch(place) {
		case EdgeTimeBuilder::IN_PREFIX:	return on ? PREFIX_ON : PREFIX_OFF;
		case EdgeTimeBuilder::IN_BLOCK:		return on ? BLOCK_ON : BLOCK_OFF;
		default:							ASSERT(false); return PREFIX_ON;
		}
	}

private:

	t::uint32 imprec;
	Event *evt;
	genstruct::SLList<ilp::Var *> vars[SIZE];
};


/**
 * @class EdgeTimeBuilder
 * Compute execution time by edge using the parametric exegraph approach.
 * Notice that only two times are considered by edge: worst time and best time.
 * Possible execution are classified in two sets, worst and best, and max of these
 * sets represent the worst and the best time. The repartition is made to maximize
 * the gap between max of best set and min of worst set.
 *
 * @par Provided Features
 * @li @ref ipet::BB_TIME_FEATURE
 *
 * @par Required Features
 *
 * @ingroup etime
 */


p::declare EdgeTimeBuilder::reg = p::init("otawa::etime::EdgeTimeBuilder", Version(1, 0, 0))
	.base(GraphBBTime<ParExeGraph>::reg)
	.maker<EdgeTimeBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(STANDARD_EVENTS_FEATURE)
	.provide(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(EDGE_TIME_FEATURE);


/**
 */
EdgeTimeBuilder::EdgeTimeBuilder(p::declare& r): GraphBBTime<ParExeGraph>(r) {
}


void EdgeTimeBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}


/**
 */
void EdgeTimeBuilder::setup(WorkSpace *ws) {
	sys = ipet::SYSTEM(ws);
}


/**
 */
void EdgeTimeBuilder::cleanup(WorkSpace *ws) {
	for(genstruct::HashTable<Event *, EventCollector *>::Iterator coll(events); coll; coll++) {
		coll->make(sys);
		delete *coll;
	}
	events.clear();
}


/**
 */
void EdgeTimeBuilder::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;
	for(BasicBlock::InIterator edge(bb); edge; edge++)
		processEdge(ws, cfg, edge);
}


/**
 * This method is called to build the parametric execution graph.
 * As a default, build a usual @ref ParExeGraph but it may be overriden
 * to build a custom graph.
 * @param seq	Sequence to build graph for.
 * @return		Built graph.
 */
ParExeGraph *EdgeTimeBuilder::make(ParExeSequence *seq) {
	PropList props;
	ParExeGraph *graph = new ParExeGraph(this->workspace(), _microprocessor, seq, props);
	graph->build(seq);
	return graph;
}


/**
 * Called to cleanup a graph allocated by a call to @ref make().
 * @param grapĥ		Graph to clean.
 */
void EdgeTimeBuilder::clean(ParExeGraph *graph) {
	delete graph;
}


/**
 */
void EdgeTimeBuilder::processEdge(WorkSpace *ws, CFG *cfg, Edge *edge) {
	if(logFor(Processor::LOG_BLOCK))
		log << "\t\t\t" << edge << io::endl;

	// initialize the sequence
	int index = 0;
	ParExeSequence *seq = new ParExeSequence();

	// compute source
	BasicBlock *source = edge->source();
	if(source->isEntry()) {
		if(CALLED_BY(source->cfg()).exists())
			source = CALLED_BY(source->cfg())->source();
		else
			source = 0;
	}

	// compute target
	BasicBlock *target;
	if(edge->kind() == Edge::CALL)
		target = edge->calledCFG()->firstBB();
	else
		target = edge->target();

	// fill with previous
	if(source)
		for(BasicBlock::InstIterator inst(source); inst; inst++) {
			ParExeInst * par_exe_inst = new ParExeInst(inst, source, PROLOGUE, index++);
			seq->addLast(par_exe_inst);
		}

	// fill with current block
	for(BasicBlock::InstIterator inst(target); inst; inst++) {
		ParExeInst * par_exe_inst = new ParExeInst(inst, target, BODY, index++);
		seq->addLast(par_exe_inst);
	}

	// build the graph
	PropList props;
	ParExeGraph *graph = make(seq);

	// collect and sort events
	genstruct::Vector<event_t> all_events;
	if(source)
		sortEvents(all_events, source, IN_PREFIX);
	sortEvents(all_events, target, IN_BLOCK);

	// applying static events (always, never)
	event_list_t events;
	genstruct::Vector<ParExeInst *> insts;
	event_list_t always_events;
	ParExeSequence::InstIterator inst(seq);
	for(event_list_t::Iterator event(all_events); event; event++) {
		Event *evt = (*event).fst;

		// find the instruction
		while(inst->inst() != evt->inst()) {
			inst++;
			ASSERT(inst);
		}

		// apply the event
		switch(evt->occurrence()) {
		case NEVER:			continue;
		case SOMETIMES:		events.add(*event); insts.add(*inst); break;
		case ALWAYS:		apply(evt, inst); always_events.add(*event); break;
		default:		ASSERT(0); break;
		}
	}

	// check number of events limit
	// TODO	fall back: analyze the block alone, split the block
	if(events.count() >= 32)
		throw ProcessorException(*this, _ << "too many events on edge " << edge);
	if(logFor(LOG_BB))
		log << "\t\t\tevents = " << events.count() << io::endl;

	// simple trivial case
	if(events.isEmpty()) {
		ot::time cost = graph->analyze();
		sys->addObjectFunction(cost, ipet::VAR(edge));
		delete graph;
		return;
	}

	// compute all cases
	t::uint32 prev = 0;
	genstruct::Vector<ConfigSet> confs;
	for(t::uint32 mask = 0; mask < 1 << events.count(); mask++) {

		// adjust the graph
		for(int i = 0; i < events.count(); i++)
			if((prev & (1 << i)) != (mask & (1 << i))) {
				if(mask & (1 << i))
					apply(events[i].fst, insts[i]);
				else
					rollback(events[i].fst, insts[i]);
			}

		// compute and store the new value
		ot::time cost = graph->analyze();

		// add the new time
		bool done = false;
		for(int j = 0; j < confs.length(); j++)
			if(confs[j].time() == cost) {
				done = true;
				confs[j].add(Config(mask));
				break;
			}
		if(!done) {
			confs.add(ConfigSet(cost));
			confs[confs.length() - 1].add(mask);
		}
	}
	if(isVerbose())
		displayConfs(confs, events);
	delete graph;

	// trivial case: 1 time
	if(confs.length() == 1) {

		// add to the objective function
		sys->addObjectFunction(confs[0].time(), ipet::VAR(edge));

		// TODO add the edge variable to the event constraints
		return;
	}

	// split in sets LTS and HTS
	int p = splitConfs(confs);

	// build the lower set
	ot::time lts_time = confs[p - 1].time();
	ot::time hts_time = confs.top().time();
	ConfigSet hts(hts_time);
	for(int i = p; i < events.length(); i++)
		hts.add(confs[i]);
	if(isVerbose()) {
		log << "\t\t\t\t" << "LTS time = " << lts_time << ", HTS time = " << hts_time << " for { ";
		bool fst = true;
		for(ConfigSet::Iter conf(hts); conf; conf++) {
			if(fst)
				fst = false;
			else
				log << ", ";
			log << (*conf).toString(events.length());
		}
		log << " }\n";
	}

	// prepare the constraints
	t::uint32 pos = hts.posConst();
	t::uint32 neg = hts.negConst();
	t::uint32 unu = hts.unused(neg, pos, events.length());
	t::uint32 com = hts.complex(neg, pos, unu, events.length());
	if(isVerbose())
		log << "\t\t\t\t"
			<< "pos = " << maskToString(pos, events)
			<< ", neg = " << maskToString(neg, events)
			<< ", unused = " << maskToString(unu, events)
			<< ", complex = " << maskToString(com, events) << io::endl;

	// too complex case
	if(com) {
		if(isVerbose())
			log << "\t\t\t\ttoo complex: " << io::hex(com) << io::endl;

		// add to the objective function
		sys->addObjectFunction(hts_time, ipet::VAR(edge));

		// add the edge variable to the event constraints
		for(event_list_t::Iterator event(events); event; event++) {
			get((*event).fst)->contribute(EventCollector::make((*event).snd, true), 0);
			get((*event).fst)->contribute(EventCollector::make((*event).snd, false), 0);
		}
		return;
	}

	// new LTS variable
	string hts_name;
	if(_explicit) {
		StringBuffer buf;
		buf << "e_";
		if(source)
			buf << source->number() << "_";
		buf << target->number() << "_" << target->cfg()->label() << "_hts";
		hts_name = buf.toString();
	}
	ilp::Var *x_hts = sys->newVar(hts_name);

	// wcet += time_lts x_edge + (time_hts - time_lts) x_hts
	ilp::Var *x_edge = ipet::VAR(edge);
	sys->addObjectFunction(lts_time, x_edge);
	sys->addObjectFunction(hts_time - lts_time, x_hts);

	// 0 <= x_hts <= x_edge
	ilp::Constraint *cons = sys->newConstraint("0 <= x_hts", ilp::Constraint::LE);
	cons->addRight(1, x_hts);
	cons = sys->newConstraint("x_hts <= x_edge", ilp::Constraint::LE);
	cons->addLeft(1, x_hts);
	cons->addRight(1, x_edge);

	// NOTATION
	// C^e_p -- constraint of event e for position p (p in {prefix, block})
	// unprecise(C^e_p) in B (T if C^e_p unprecise e.g. use of <=, _ else and as a default)

	// foreach e in events do
	for(int i = 0; i < events.count(); i++) {

		// if e in pos_events then C^e_p += x_hts / p = prefix if e in prefix, block
		if(pos & (1 << i))
			get(events[i].fst)->contribute(EventCollector::make(events[i].snd, true), x_hts);

		// else if e in neg_events then C^e_p += x_edge - x_hts / p = prefix if e in prefix, block
		else if(neg & (1 << i))
			get(events[i].fst)->contribute(EventCollector::make(events[i].snd, false), x_hts);

		// else unprecise(C^e_p) = T / p = prefix if e in prefix, block
		else {
			get(events[i].fst)->contribute(EventCollector::make(events[i].snd, true), 0);
			get(events[i].fst)->contribute(EventCollector::make(events[i].snd, false), 0);
		}
	}

	// foreach e in always(e) do C^e_p += x_edge
	for(event_list_t::Iterator event(all_events); event; event++)
		switch((*event).fst->occurrence()) {
		case ALWAYS:	get((*event).fst)->contribute(EventCollector::make((*event).snd, true), x_hts); break;
		case NEVER:		get((*event).fst)->contribute(EventCollector::make((*event).snd, false), x_hts); break;
		default:		break;
		}
}


/**
 * Get the collector for the given event.
 * If it doesn't exist, create it.
 * @param event		Concerned event.
 * @return			Matching collector (never null).
 */
EventCollector *EdgeTimeBuilder::get(Event *event) {
	EventCollector *coll = events.get(event, 0);
	if(!coll) {
		coll = new EventCollector(event);
		events.put(event, coll);
	}
	return coll;
}


/**
 * Sort events according the instructions they apply to.
 * @param events	Data structure to store events to.
 * @param bb		BasicBlock to look events in.
 * @param place		Place in the sequence.
 */
void EdgeTimeBuilder::sortEvents(event_list_t& events, BasicBlock *bb, place_t place) {
	typedef avl::Set<Event *, EventComparator> set_t;
	set_t set;
	for(Identifier<Event *>::Getter event(bb, EVENT); event; event++)
		set.add(event);
	for(set_t::Iterator e(set); e; e++)
		events.push(pair(*e, place));
}


/**
 * Partition the configuration times in two sets: configuration times
 * in [0, p[ are the low time set (LTS) and the configuration times in
 * [p, ...] are the high time set (HTS).
 * @param confs	Configuration set to find partition for.
 * @return		Position of partition in confs.
 */
int EdgeTimeBuilder::splitConfs(genstruct::Vector<ConfigSet>& confs) {
	static const float	sep_factor = 1,			// TODO: put them in configuration
						over_factor = 1.5;

	// initialization
	int p = 1, best_p = 1;
	float best_cost = type_info<float>::min;
	int min_low = confs[0].time(), max_low = confs[0].time(), cnt_low = confs[0].count();
	int min_high = confs[1].time(), max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// computation
	while(p < confs.length()) {

		// select the best split
		float cost = sep_factor * (max_low - min_high) + over_factor * (max_low - min_low);
		if(cost > best_cost) {
			best_p = p;
			best_cost = cost;
		}

		// prepare next split
		max_low = confs[p].time();
		p++;
		if(p < confs.length())
			min_high = confs[p].time();
	}
	return best_p;
}


/**
 * Display the list of configuration sorted by cost.
 * @param confs		List of configuration to displayy.
 */
void EdgeTimeBuilder::displayConfs(const genstruct::Vector<ConfigSet>& confs, const event_list_t& events) {
	for(int i = 0; i < confs.length(); i++) {
		log << "\t\t\t\t" << confs[i].time() << " -> ";
		for(ConfigSet::Iter conf(confs[i]); conf; conf++)
			log << " " << (*conf).toString(events.length());
		log << io::endl;
	}
}


/**
 * Apply the given event to the given instruction.
 * @param event		Event to apply.
 * @param inst		Instruction to apply to.
 */
void EdgeTimeBuilder::apply(Event *event, ParExeInst *inst) {

	switch(event->kind()) {

	case FETCH:
		inst->fetchNode()->setLatency(inst->fetchNode()->latency() + event->cost());
		break;

	default:
		ASSERTP(0, _ << "unsupported event kind " << event->kind());
		break;
	}

}

/**
 * Rollback the given event to the given instruction.
 * @param event		Event to apply.
 * @param inst		Instruction to apply to.
 */
void EdgeTimeBuilder::rollback(Event *event, ParExeInst *inst) {

	switch(event->kind()) {

	case FETCH:
		inst->fetchNode()->setLatency(inst->fetchNode()->latency() - event->cost());
		break;

	default:
		ASSERTP(0, _ << "unsupported event kind " << event->kind());
		break;
	}

}


/**
 * This feature ensures that block cost has been computed according to the context
 * of edges. Basically, this means that both the objective function and the constraint
 * of events has been added to the ILP system.
 */
p::feature EDGE_TIME_FEATURE("otawa::etime::EDGE_TIME_FEATURE", new Maker<EdgeTimeBuilder>());

} }	// otawa::etime
