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
#include <elm/data/quicksort.h>
#include <otawa/etime/Config.h>
#include <otawa/etime/EventCollector.h>

namespace otawa { namespace etime {

io::Output& operator<<(io::Output& out, EdgeTimeBuilder::place_t p) {
	static cstring msgs[] = {
			"in prefix",
			"in edge",
			"in block"
	};
	ASSERT(p >= EdgeTimeBuilder::IN_PREFIX && p < EdgeTimeBuilder::IN_SIZE);
	out << msgs[p];
	return out;
}


/**
 * @class EdgeTimeGraph
 * Execution graph class to customize the debug output with events.
 * @ingroup etime
 */

/**
 */
EdgeTimeGraph::EdgeTimeGraph(
	WorkSpace * ws,
	ParExeProc *proc,
	Vector<Resource *> *hw_resources,
	ParExeSequence *seq,const PropList& props)
: ParExeGraph(ws, proc, hw_resources, seq, props), builder(0) {
}

template <class T, class U>
inline io::Output& operator<<(io::Output& out, const Pair<T, U>& p)
	{ out << "(" << p.fst << ", " << p.snd << ")"; return out; }

/**
 */
void EdgeTimeGraph::customDump(io::Output& out) {
	out << "{";
	out << "events|";
	bool first = true;
	for(int i = 0; i < builder->all_events.count(); i++) {

		// display separator
		if(first)
			first = false;
		else
			out << "\\l";

		// define activation
		int bit = -1;
		for(int j = 0; j < builder->events.count(); j++)
			if(builder->events[j] == builder->all_events[i]) {
				bit = j;
				break;
			}
		if(bit < 0)
			out << "\\[*\\] ";
		else if(builder->event_mask & (1 << bit))
			out << "\\[1\\] ";
		else
			out << "\\[-\\] ";

		// display event
		out << builder->all_events[i].fst->inst()->address() << " " << builder->all_events[i].fst->detail();
	}
	out << "\\l}";
}


/**
 * @class Config
 * Represents a configuration for computing cost of a BB.
 * Each bits represents the state of an event:
 * @li 0	event not activated
 * @li 1	event activated
 */


/**
 * Convert config to string.
 * @param n		Number of bits in the configuration.
 * @return		String display of the configuration.
 */
string Config::toString(int n) const {
	StringBuffer buf;
	for(int i = 0; i < n; i++)
		buf << (bit(i) ? char(i + 'a') : '-');
	return buf.toString();
}


/**
 * @class ConfigSet
 * Set of configurations with the same or with a maximum of time.
 */



/**
 * TODO
 */
t::uint32 ConfigSet::posConst(void) const {
	t::uint32 r = 0xffffffff;
	for(int i = 0; i < confs.length(); i++)
		r &= confs[i].bits();
	return r;
}


/**
 * TODO
 */
t::uint32 ConfigSet::negConst(void) const {
	t::uint32 r = 0;
	for(int i = 0; i < confs.length(); i++)
		r |= confs[i].bits();
	return ~r;
}


/**
 * TODO
 */
t::uint32 ConfigSet::unused(t::uint32 neg, t::uint32 pos, int n) const {
	t::uint32 mask = neg | pos;
	t::uint32 r = 0;
	for(int i = 0; i < n; i++)
		if(!(mask & (1 << i))) {
			Vector<t::uint32> nconf, pconf;
			for(int j = 0; j < confs.length(); j++) {
				t::uint32 c = confs[j].bits();
				if(c & (1 << i)) {
					c &= ~(1 << i);
					if(nconf.contains(c))
						nconf.remove(c & ~(1 << i));
					else
						pconf.add(c & ~(1 << i));
				}
				else {
					if(pconf.contains(c))
						pconf.remove(c);
					else
						nconf.add(c);
				}
			}
			if(!pconf && !nconf) {
				r |= 1 << i;
			}
		}
	return r;
}


/**
 * TODO
 */
t::uint32 ConfigSet::complex(t::uint32 neg, t::uint32 pos, t::uint32 unused, int n) const {
	return ((1 << n) - 1) & ~neg & ~pos & ~unused;
}


/**
 * TODO
 */
void ConfigSet::scan(t::uint32& pos, t::uint32& neg, t::uint32& unus, t::uint32& comp, int n) const {
	pos = posConst();
	neg = negConst();
	unus = unused(neg, pos, n);
	comp = complex(neg, pos, unus, n);
}


/**
 * Test if the configuration set is feasible.
 */
bool ConfigSet::isFeasible(int n) {
	t::uint32 pos, neg, unus, comp;
	scan(neg, pos, unus, comp, n);
	return !comp;
}


/**
 * TODO
 */
void ConfigSet::dump(io::Output& out, int n) {
	bool fst = true;
	out << "{ ";
	for(int i = 0; i < confs.length(); i++) {
		if(fst)
			fst = false;
		else
			out << ", ";
		out << confs[i].toString(n);
	}
	out << " }";
}


/**
 * Convert an event mask to a string.
 * @param mask		Event mask.
 * @param len		Number of events.
 * @return			String representing the event mask.
 */
string maskToString(t::uint32 mask, int len) {
	StringBuffer buf;
	for(int i = 0; i < len; i++)
		buf << ((mask & (1 << i)) ? char(i + 'a') : '_');
	return buf.toString();
}


/**
 * @class EventCollector
 * Collects variables linking events with blocks in ILP.
 */


/**
 * Add a variable contribution.
 * @param c		Case of contribution.
 * @param var	Variable of contribution (null for blurred contribution driving to estimation).
 */
void EventCollector::contribute(case_t c, ilp::Var *var) {
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
void EventCollector::make(ilp::System *sys) {
	for(int c = 0; c < SIZE; ++c) {
		if(!vars[c].isEmpty() && evt->isEstimating(isOn(case_t(c)))) {
			ilp::Constraint *cons = sys->newConstraint(evt->name() ,
				/*(imprec & (1 << c)) ?*/ ilp::Constraint::GE /*: ilp::Constraint::EQ*/);
			evt->estimate(cons, isOn(case_t(c)));
			for(List<ilp::Var *>::Iter v(vars[c]); v(); v++) {
				ASSERT(*v != nullptr);
				cons->addRight(1, *v);
			}
		}
	}
}


/**
 * Test if the event is on or off.
 * @param c		Case to test.
 * @return		True if on, false if off.
 */
inline bool EventCollector::isOn(case_t c) {
	static bool ons[SIZE] = { false, true, false, true };
	return ons[c];
}


/**
 * Build an event selector case.
 */
EventCollector::case_t EdgeTimeBuilder::make(const Event *e, EdgeTimeBuilder::place_t place, bool on) {
	switch(place) {
	case EdgeTimeBuilder::IN_PREFIX:	return on ? EventCollector::PREFIX_ON : EventCollector::PREFIX_OFF;
	case EdgeTimeBuilder::IN_BLOCK:		return on ? EventCollector::BLOCK_ON : EventCollector::BLOCK_OFF;
	case EdgeTimeBuilder::IN_EDGE:		return on ? EventCollector::BLOCK_ON : EventCollector::BLOCK_OFF;
	default:							ASSERT(false); return EventCollector::PREFIX_ON;
	}
}


/**
 * @class EdgeTimeBuilder
 * Compute execution time by edge using the parametric exegraph approach.
 * Notice that only two times are considered by edge: worst time and best time.
 * Possible execution are classified in two sets, worst and best, and max of these
 * sets represent the worst and the best time. The repartition is made to maximize
 * the gap between max of best set and min of worst set.
 *
 * @par Provided Features
 * @li @ref etime::EDGE_TIME_FEATURE
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 *
 * @par Required Features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref WEIGHT_FEATURE
 * @li @ref EVENTS_FEATURE
 *
 * @ingroup etime
 */


p::declare EdgeTimeBuilder::reg = p::init("otawa::etime::EdgeTimeBuilder", Version(1, 0, 0))
	.base(GraphBBTime<ParExeGraph>::reg)
	.maker<EdgeTimeBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(WEIGHT_FEATURE)
	.require(EVENTS_FEATURE)
	.require(hard::PROCESSOR_FEATURE)
	.provide(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(EDGE_TIME_FEATURE);


/**
 */
EdgeTimeBuilder::EdgeTimeBuilder(p::declare& r)
:	GraphBBTime<EdgeTimeGraph>(r),
 	_explicit(false),
 	sys(0),
 	predump(false),
 	event_th(0),
 	edge(0),
 	seq(0),
 	graph(0),
 	bnode(0),
 	bedge(0),
 	source(0),
 	target(0),
	record(false),
	event_mask(0)
{ }


void EdgeTimeBuilder::configure(const PropList& props) {
	GraphBBTime<EdgeTimeGraph>::configure(props);
	_explicit = ipet::EXPLICIT(props);
	predump = PREDUMP(props);
	event_th = EVENT_THRESHOLD(props);
	record = RECORD_TIME(props);
	_props = props;
}


/**
 */
void EdgeTimeBuilder::setup(WorkSpace *ws) {
	sys = ipet::SYSTEM(ws);

}


/**
 */
void EdgeTimeBuilder::cleanup(WorkSpace *ws) {
	for(HashMap<Event *, EventCollector *>::Iter coll(colls); coll(); coll++) {
		coll->make(sys);
		delete *coll;
	}
	events.clear();
}


/**
 */
void EdgeTimeBuilder::processBB(WorkSpace *ws, CFG *cfg, Block *b) {

	// nothing to with an end or a synthetic block
	if(!b->isBasic())
		return;
	target = b->toBasic();

	// collect all predecessors BBs
	BasicBlock::basic_preds_t ps;
	target->basicPreds(ps);

	// compute time for each case
	for(auto p: ps) {
		source = p.fst;
		edge = p.snd;
		processEdge(ws, cfg);
	}

#if 0
	// look out all predecessors
	for(auto e: b->inEdges()) {
		source = nullptr;
		Block *p = e->source();

		// find the previous BB
		while(source == nullptr) {

			// BB case
			if(p->isBasic())
				source = p->toBasic();

			// phony case
			else if(p->isPhony()) {
				if(p->countIns() == 1)
					p = p->inEdges().begin()->source();
				else
					break;
			}

			// entry case
			else if(p->isEntry()) {
				if(p->cfg()->callCount() == 1)
					p = p->cfg()->callers().begin()->inEdges().begin()->source();
				else
					break;
			}

			// synthetic case
			else {
				SynthBlock *sb = p->toSynth();
				if(sb->callee() != nullptr
				&& sb->callee()->exit()->countIns() == 1)
					p = sb->callee()->exit()->inEdges().begin()->source();
				else
					break;
			}

		}

		// compute the time
		edge = e;
		processEdge(ws, cfg);
	}
#endif
#if 0
	// use each basic edge
	bool one = false;
	for(BasicBlock::BasicIns e(bb); e; e++) {
		source = (*e).source();
		edge = (*e).edge();
		target = (*e).sink();
		processEdge(ws, cfg);
		one = true;
	}

	// first block: no predecessor
	if(!one) {
		source = nullptr;
		edge = nullptr;
		target = bb;
		processEdge(ws, cfg);
	}
#endif

#if 0
	// process each primary edge
	for(Block::EdgeIter in = bb->ins(); in; in++) {
		typedef Vector<Pair<BasicBlock *, Edge *> > comps_t;
		comps_t comps;
		Vector<Edge *> todo;

		// close all predecessor BB
		todo.add(in);
		while(todo) {
			Edge *e = todo.pop();
			if(e->source()->isBasic())
				comps.add(pair(e->source()->toBasic(), e));
			else if(e->source()->isEntry())
				for(CFG::CallerIter c = cfg->callers(); c; c++)
					for(Block::EdgeIter ce = c->ins(); ce; ce++)
						todo.push(ce);
			else {
				SynthBlock *sb = e->source()->toSynth();
				if(!sb->callee() || count(sb->callee()->callers()) > 1) {
					comps.clear();
					break;
				}
				for(Block::EdgeIter ce = sb->callee()->exit()->ins(); ce; ce++)
					todo.push(ce);
			}
		}

		// perform the computations
		if(!comps) {
			source = 0;
			edge = in;
			target = bb;
			processEdge(ws, cfg);
		}
		else
			for(comps_t::Iterator comp(comps); comp; comp++) {
				source = (*comp).fst;
				edge = (*comp).snd;
				target = bb;
				processEdge(ws, cfg);
			}
	}
#	endif
}


/**
 * This method is called to build the parametric execution graph.
 * As a default, build a usual @ref ParExeGraph but it may be overridden
 * to build a custom graph.
 * @param seq	Sequence to build graph for.
 * @return		Built graph.
 */
EdgeTimeGraph *EdgeTimeBuilder::make(ParExeSequence *seq) {
	EdgeTimeGraph *graph = new EdgeTimeGraph(this->workspace(), _microprocessor, &_hw_resources, seq, _props);
	if(_do_output_graphs)
		graph->setExplicit(true);
	graph->build();
	return graph;
}


/**
 * Called to cleanup a graph allocated by a call to @ref make().
 * @param grap	Graph to clean.
 */
void EdgeTimeBuilder::clean(ParExeGraph *graph) {
	delete graph;
}


/**
 */
void EdgeTimeBuilder::processEdge(WorkSpace *ws, CFG *cfg) {
	if(logFor(Processor::LOG_BLOCK))
		log << "\t\t\t" << source << ", " << edge << ", " << target << io::endl;

	// initialize the sequence
	bnode = 0;
	seq = new ParExeSequence();

	// collect and sort events
	all_events.clear();
	if(source)
		sortEvents(all_events, source, IN_PREFIX, edge);
	sortEvents(all_events, target, IN_BLOCK, edge);

	// usual simple case: few events
	if(countDynEvents(all_events) <= event_th) {
		int index = 0;

		// fill the prefix
		if(source)
			for(BasicBlock::InstIter inst = source->insts(); inst(); inst++) {
				ParExeInst * par_exe_inst = new ParExeInst(*inst, source, PROLOGUE, index++);
				seq->addLast(par_exe_inst);
			}

		// fill the current block
		for(BasicBlock::InstIter inst = target->insts(); inst(); inst++) {
			ParExeInst * par_exe_inst = new ParExeInst(*inst, target, otawa::BLOCK, index++);
			seq->addLast(par_exe_inst);
		}

		// perform the computation
		processSequence();
		delete seq;
		return;
	}

	// remove prefix case
	if(logFor(LOG_BLOCK))
		log << "\t\t\ttoo many dynamic events (" << countDynEvents(all_events) << "). Discarding prefix: overestimation increases.\n";
	all_events.clear();
	sortEvents(all_events, target, IN_BLOCK, edge);
	if(countDynEvents(all_events) <= event_th) {

		// fill the current block
		int index = 0;
		for(BasicBlock::InstIter inst(target); inst(); inst++) {
			ParExeInst * par_exe_inst = new ParExeInst(*inst, target, otawa::BLOCK, index++);
			seq->addLast(par_exe_inst);
		}

		// perform the computation
		processSequence();
		delete seq;
		return;
	}

#if 0
	// for now, just crash
	else {
		if(logFor(LOG_BLOCK)) {
			log << "\t\t\ttoo many dynamic events: " << countDynEvents(all_events) << ". Giving up. Sorry.\n";
			// log used events
			for(Vector<event_t>::Iter e(all_events); e(); e++)
				if((*e).fst->occurrence() == SOMETIMES)
					log << "\t\t\t\t" << (*e).snd << ": " << (*e).fst->inst()->address() << " -> " << (*e).fst->name() << " (" << (*e).fst->detail() << ") " << (*e).snd << io::endl;
		}
		ASSERTP(false, "too many events! (" << countDynEvents(all_events) << ")")
	}
#endif

	// find bounds in event list
	event_list_t events = all_events;
	Vector<Inst *> bounds;
	int dyn_cnt = 0, inst_cnt = 0;
	Inst *inst = target->first();
	for(int i = 0; i < events.length(); i++) {
		if(events[i].fst->inst() != inst) {
			inst = events[i].fst->inst();
			inst_cnt = 0;
		}
		if(events[i].fst->occurrence() == SOMETIMES) {
			dyn_cnt++;
			inst_cnt++;
		}
		if(dyn_cnt > event_th && dyn_cnt != inst_cnt) {
			if(logFor(LOG_BLOCK))
				log << "\t\t\tsub-BB ending at " << inst->address()
					<< ": " << (dyn_cnt - inst_cnt) << " events\n";
			bounds.add(inst);
			dyn_cnt = inst_cnt;
		}
	}
	if(logFor(LOG_BLOCK))
		log << "\t\t\tsub-BB ending at end: " << dyn_cnt << " events\n";

	// process the different sub-blocks
	int bi = 0, ei = 0, ni = 0;
	all_events.clear();
	for(const auto inst: *target) {

		// process limit
		if(bi < bounds.length() && inst == bounds[bi]) {
			ASSERT(seq->length() != 0);
			processSequence();
			all_events.clear();
			delete seq;
			seq = new ParExeSequence();
			ni = 0;
			bi++;
		}

		// process current instruction
		ParExeInst * par_exe_inst = new ParExeInst(inst, target, otawa::BLOCK, ni++);
		seq->addLast(par_exe_inst);
		while(ei < events.length() && events[ei].fst->inst() == inst) {
			all_events.add(events[ei]);
			ei++;
		}
	}

	// compute for last block
	processSequence();
	delete seq;
}


/**
 * Count the number of variable events in the event list.
 * @param events	Event list to process.
 * @return			Number of variable events.
 */
int EdgeTimeBuilder::countDynEvents(const event_list_t& events) {
	int cnt = 0;
	for(int i = 0; i < events.length(); i++)
		if(events[i].fst->occurrence() == SOMETIMES)
			cnt++;
	return cnt;
}


/**
 * Compute and process the time for the given sequence.
 */
void EdgeTimeBuilder::processSequence(void) {

	// log used events
	if(logFor(LOG_BB))
		for(Vector<event_t>::Iter e(all_events); e(); e++)
			log << "\t\t\t\t" << (*e).snd << ": " << (*e).fst->inst()->address()
				<< " -> " << (*e).fst->name() << " (" << (*e).fst->detail() << ") "
				<< (*e).snd << io::endl;

	// build the graph
	PropList props;
	graph = make(seq);
	graph->setBuilder(*this);
	ASSERTP(graph->firstNode(), "no first node found: empty execution graph");

	// applying static events (always, never)
	events.clear();
	Vector<ParExeInst *> insts;
	event_list_t always_events;
	ParExeSequence::InstIterator inst(seq);
	for(event_list_t::Iter event(all_events); event(); event++) {
		Event *evt = (*event).fst;

		// find the instruction
		while((*event).snd != IN_PREFIX && inst->codePart() != otawa::BLOCK) {
			inst++;
			ASSERT(inst);
		}
		while(inst->inst() != evt->inst()) {
			inst++;
			ASSERTP(inst, "no instruction for event " << evt->inst()->address() << ":" << evt->inst());
		}

		// apply the event
		switch(evt->occurrence()) {
		case NEVER:			continue;
		case SOMETIMES:		events.add(*event); insts.add(*inst); break;
		case ALWAYS:		apply(evt, *inst); always_events.add(*event); break;
		default:			ASSERT(0); break;
		}
	}

	if(logFor(LOG_BB)) {
		log << "\t\t\t\tdynamic events = " << events.count() << io::endl;
		for(int i = 0; i < events.count(); i++)
			log << "\t\t\t\t(" << char(i + 'a') << ") " << events[i].fst->inst()->address() << "\t" << events[i].fst->name()
				<< " " << events[i].snd << io::endl;
	}

	// check number of events limit
	// TODO	fall back: analyze the block alone, split the block
	if(events.count() >= 32)
		throw ProcessorException(*this, _ << "too many events on edge " << edge);

	// simple trivial case
	if(events.isEmpty()) {

		// predump implementation
		if(_do_output_graphs && predump)
			outputGraph(graph, 666, 666, 666, _ << source << " -> " << target);

		// analyze
		ot::time cost = graph->analyze();
		genForOneCost(cost, edge, events);

		// dump it if needed
		if(_do_output_graphs) {
			if (source)
				outputGraph(graph, target->index(), source->index(), 0,
						_ << source << " -> " << target << " (cost = " << cost << ")");
			else
				outputGraph(graph, target->index(), 0, 0, _ << target << " (cost = " << cost << ")");
		}
		delete graph;
		return;
	}

	// compute all cases
	t::uint32 prev = 0;
	Vector<ConfigSet> confs;
	custom.clear();
	for(event_mask = 0; event_mask < t::uint32(1 << events.count()); event_mask++) {

		// adjust the graph
		for(int i = 0; i < events.count(); i++) {
			if((prev & (1 << i)) != (event_mask & (1 << i))) {
				if(event_mask & (1 << i))
					apply(events[i].fst, insts[i]);
				else
					rollback(events[i].fst, insts[i]);
			}
		}
		prev = event_mask;

		// predump implementation
		if(_do_output_graphs && predump)
			outputGraph(graph, 666, 666, 666, _ << source << " -> " << target);

		// compute and store the new value
		ot::time cost = graph->analyze();

		// dump it if needed
		if(_do_output_graphs) {
			if (source)
				outputGraph(graph, target->index(), source->index(), event_mask,
						_ << source << " -> " << target << " (cost = " << cost << ")");
			else
				outputGraph(graph, target->index(), 0, event_mask, _ << target << " (cost = " << cost << ")");
		}

		// add the new time
		int j;
		for(j = 0; j < confs.length(); j++)
			if(cost == confs[j].time())
				break;
			else if(cost < confs[j].time()) {
				confs.insert(j, ConfigSet(cost));
				break;
			}
		if(j >= confs.length())
			confs.add(ConfigSet(cost));
		confs[j].add(Config(event_mask));
	}

	//if(isVerbose())
	if(logFor(LOG_BB))
		displayConfs(confs, events);
	delete graph;

	// trivial case: 1 time
	if(confs.length() == 1) {
		genForOneCost(confs[0].time(), edge, all_events);
		return;
	}

	// generate constraints
	processTimes(confs);
}


/**
 * Generate the constraints when only one cost is considered for the edge.
 * @param cost		Edge cost.
 * @param edge		Current edge.
 * @param events	List of edge events.
 */
void EdgeTimeBuilder::genForOneCost(ot::time cost, Edge *edge, event_list_t& events) {
	ilp::Var *var = ipet::VAR(edge);
	ASSERT(var);

	// logging
	if(logFor(LOG_BB))
		log << "\t\t\t\tcost = " << cost << io::endl;

	// add to the objective function
	sys->addObjectFunction(cost, var);
	if(record)
		LTS_TIME(edge) = cost;

	// generate constant contrubtion
	contributeConst();

	// generate variable contribution
	for(event_list_t::Iter event(events); event(); event++)
		if((*event).fst->occurrence() == SOMETIMES) {
			get((*event).fst)->contribute(make((*event).fst, (*event).snd, true), 0);
			get((*event).fst)->contribute(make((*event).fst, (*event).snd, false), 0);
		}
}


/**
 * Get the collector for the given event.
 * If it doesn't exist, create it.
 * @param event		Concerned event.
 * @return			Matching collector (never null).
 */
EventCollector *EdgeTimeBuilder::get(Event *event) {
	EventCollector *coll = colls.get(event, 0);
	if(!coll) {
		coll = new EventCollector(event);
		colls.put(event, coll);
	}
	return coll;
}


/**
 * Sort events according the instructions they apply to.
 * @param events	Data structure to store events to.
 * @param bb		BasicBlock to look events in.
 * @param place		Place in the sequence.
 * @param edge		Edge events to include.
 */
void EdgeTimeBuilder::sortEvents(event_list_t& events, BasicBlock *bb, place_t place, Edge *edge) {
	typedef avl::Set<event_t, EventComparator> set_t;
	set_t set;

	// events in the current block
	for(Identifier<Event *>::Getter event(bb, EVENT); event(); event++)
		set.add(pair(*event, place));

	// collect events on edge
	if(place == IN_PREFIX)
		for(Identifier<Event *>::Getter event(edge, PREFIX_EVENT); event(); event++)
			set.add(pair(*event, IN_PREFIX));
	else
		for(Identifier<Event *>::Getter event(edge, EVENT); event(); event++)
			set.add(pair(*event, IN_EDGE));

	// generate the ordered list of events
	events.addAll(set);
}


/**
 * Partition the configuration times in two sets: configuration times
 * in [0, p[ are the low time set (LTS) and the configuration times in
 * [p, ...] are the high time set (HTS).
 * @param confs		Configuration set to find partition for.
 * @param events	List of events.
 * @return			Position of partition in confs.
 */
int EdgeTimeBuilder::splitConfs(const config_list_t& confs, const event_list_t& events, bool& lower) {
	static const float	sep_factor = 1,			// TODO: put them in configuration
						span_factor = 1;

	// initialization
	int p = 1, best_p = -1;
	float best_cost = type_info<float>::min;
	int min_low = confs[0].time(), max_low = confs[0].time();	// , cnt_low = confs[0].count();
	int min_high = confs[1].time();								// , max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);

	// computation
	for(p = 1; p < confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);
		min_high = confs[p - 1].time();
		max_low = confs[p].time();

		// select the best split
		float cost = sep_factor * (min_high - max_low) - span_factor * (max_low - min_low);
		if(cost > best_cost && set.isFeasible(events.length())) {
			best_p = p;
			best_cost = cost;
		}

		// prepare next split
	}
	return best_p;
}


/**
 * Display the list of configuration sorted by cost.
 * @param confs		List of configuration to display.
 */
void EdgeTimeBuilder::displayConfs(const Vector<ConfigSet>& confs, const event_list_t& events) {
	if(confs) {
		for(int i = 0; i < confs.length(); i++) {
			log << "\t\t\t\t[" << i << "] cost = " << confs[i].time() << " with " << confs[i].count() << " confs" << " -> ";
			for(ConfigSet::Iter conf(confs[i]); conf(); conf++)
				log << " " << (*conf).toString(events.length());
			log << io::endl;
		}
	}
}


/**
 * Get the branch node resolving a branch prediction.
 * @return	Node of resolution.
 */
ParExeNode *EdgeTimeBuilder::getBranchNode(void) {
	ASSERT(source);
	if(!bnode) {
		Inst *binst = source->control();
		for(ParExeSequence::Iter pinst(*seq); pinst(); pinst++)
			if(pinst->inst() == binst) {
				for(ParExeInst::NodeIterator node(*pinst); node(); node++)
					if(node->stage()->unit()->isBranch()) {
						bnode = *node;
						break;
					}
				if(!bnode)
					bnode = pinst->fetchNode();
				break;
			}
	}
	return bnode;
}


/**
 * Apply the given event to the given instruction.
 * @param event		Event to apply.
 * @param inst		Instruction to apply to.
 */
void EdgeTimeBuilder::apply(Event *event, ParExeInst *inst) {
	static string pred_msg = "pred";

	switch(event->kind()) {

	case FETCH: {
		switch (event->type()) {
		case LOCAL:
			inst->fetchNode()->setLatency(inst->fetchNode()->latency() + event->cost());
			break;

		case AFTER:
		case NOT_BEFORE: {
			// Edge type depends on the event type
			ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
					ParExeEdge::SOLID : ParExeEdge::SLASHED;

			// Find the related ParExeInst
			ParExeInst *rel_inst = 0;
			for(ParExeSequence::Iter inst_it(*seq); inst_it(); ++inst_it)
				if(inst_it->inst() == event->related().fst) {
					rel_inst = *inst_it;
					break;
				}
			ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
					<< ") not found in the sequence");

			// Find the related ParExeNode
			bool found = false;
			for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node(); rel_node++)
				if(rel_node->stage()->unit() == event->related().snd) {
					found = true;

					// Add cost to the edge between the related node and the fetch code
					bool edge_found = false;
					for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ)
						if (*succ == inst->fetchNode() && succ.edge()->type() == edge_type) {
							succ.edge()->setLatency(succ.edge()->latency() + event->cost());
							edge_found = true;
							break;
						}
					if (!edge_found)
						throw otawa::Exception(_ << "edge from related stage "
								<< event->related().snd << " not found");

					break;
				}
			if(!found)
				throw otawa::Exception(_ << "related stage " << event->related().snd << " not found");
			break;
		}

		default:
			ASSERTP(0, _ << "unsupported event type " << event->type());
		}

		break;
	}

	case MEM: {
		bool found = false;
		int mem_stage = 0;
		for(ParExeInst::NodeIterator node(inst); node(); node++)
			if(node->stage()->unit()->isMem()) {
				if(node->stage()->unit()->memStage() != mem_stage) {
					mem_stage++;
					continue;
				}
				node->setLatency(node->latency() + event->cost() - 1);
				found = true;
				break;
			}

		if(!found && event->related().fst) {
			ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
			ParExeInst *rel_inst = 0;
			for(ParExeSequence::Iter inst_it(*seq); inst_it(); ++inst_it) {
				if(inst_it->inst() == event->related().fst) {
					rel_inst = *inst_it;
					break;
				}
			}
			// ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

			for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node(); rel_node++)
				if(rel_node->stage()->unit() == event->related().snd) {
					found = true;

					// Add cost to the edge between the related node and the fetch code
					IN_ASSERT(bool edge_found = false);
					for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ) {
						if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
							succ.edge()->setLatency(succ.edge()->latency() + event->cost());
							IN_ASSERT(edge_found = true);
							break;
						}
					}
					ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
					break;
				}
		}

		if(!found && inst->execNode()) {
			inst->execNode()->setLatency(inst->execNode()->latency() + event->cost() - 1);
			found = true;
		}

		if(!found)
			throw otawa::Exception("no memory stage / FU found in this pipeline");
		break;
	}

	case BRANCH:
		{
			auto b = getBranchNode();
			if(b != nullptr) {
				bedge =  new ParExeEdge(getBranchNode(), inst->fetchNode(), ParExeEdge::SOLID, 0, pred_msg);
				bedge->setLatency(event->cost());
			}
			else {
				addLatency(inst->fetchNode(), event->cost());
				bedge = nullptr;
			}
		}
		break;

	case CUSTOM:
		switch(event->type()) {
		case LOCAL: {
				ParExeNode *n = findNode(inst, event->unit());
				ASSERT(n);
				addLatency(n, event->cost());
			}
			break;
		case AFTER: {
				ParExeNode *n = findNode(inst, event->unit());
				ParExeNode *m = findNode(event->related(), inst);
				if(m != nullptr) {
					ParExeEdge *e = new ParExeEdge(m, n, ParExeEdge::SOLID, event->cost(), event->name());
					custom.put(event, e);
				}
			}
			break;
		case NOT_BEFORE: {
				ParExeNode *n = findNode(inst, event->unit());
				ParExeNode *m = findNode(event->related(), inst);
				ParExeEdge *e = new ParExeEdge(m, n, ParExeEdge::SLASHED, event->cost(), event->name());
				custom.put(event, e);
			}
			break;
		}
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

	case FETCH: {
		switch (event->type()) {
		case LOCAL:
			inst->fetchNode()->setLatency(inst->fetchNode()->latency() - event->cost());
			break;

		case AFTER:
		case NOT_BEFORE: {
			// Edge type depends on the event type
			ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
					ParExeEdge::SOLID : ParExeEdge::SLASHED;

			// Find the related ParExeInst
			ParExeInst *rel_inst = 0;
			for(ParExeSequence::Iter inst_it(*seq); inst_it(); ++inst_it)
				if(inst_it->inst() == event->related().fst) {
					rel_inst = *inst_it;
					break;
				}
			ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
					<< ") not found in the sequence");

			// Find the related ParExeNode
			bool found = false;
			for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node(); rel_node++)
				if(rel_node->stage()->unit() == event->related().snd) {
					found = true;

					// Add cost to the edge between the related node and the fetch code
					bool edge_found = false;
					for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ)
						if (*succ == inst->fetchNode() && succ.edge()->type() == edge_type) {
							succ.edge()->setLatency(succ.edge()->latency() - event->cost());
							edge_found = true;
							break;
						}
					if (!edge_found)
						throw otawa::Exception(_ << "edge from related stage "
								<< event->related().snd << " not found");

					break;
				}
			if(!found)
				throw otawa::Exception(_ << "related stage " << event->related().snd << " not found");
			break;
		}

		default:
			ASSERTP(0, _ << "unsupported event type " << event->type());
		}
		break;
	}

	case MEM: {
		bool found = false;
		int mem_stage = 0;
		for(ParExeInst::NodeIterator node(inst); node(); node++)
			if(node->stage()->unit()->isMem()) {
				if(node->stage()->unit()->memStage() != mem_stage) {
					mem_stage++;
					continue;
				}
				node->setLatency(node->latency() - event->cost() + 1);
				found = true;
				break;
			}

		if(!found && event->related().fst) {
			ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
			ParExeInst *rel_inst = 0;
			for(ParExeSequence::Iter inst_it(*seq); inst_it(); ++inst_it) {
				if(inst_it->inst() == event->related().fst) {
					rel_inst = *inst_it;
					break;
				}
			}
			ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

			for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node(); rel_node++)
				if(rel_node->stage()->unit() == event->related().snd) {
					found = true;

					// Add cost to the edge between the related node and the fetch code
					IN_ASSERT(bool edge_found = false);
					for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ) {
						if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
							succ.edge()->setLatency(succ.edge()->latency() - event->cost());
							IN_ASSERT(edge_found = true);
							break;
						}
					}
					ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
					break;
				}
		}

		if(!found && inst->execNode()) {
			inst->execNode()->setLatency(inst->execNode()->latency() - event->cost() + 1);
			found = true;
		}
		break;
	}

	case BRANCH:
		if(bedge != nullptr) {
			graph->remove(bedge);
			bedge = 0;
		}
		else
			removeLatency(inst->fetchNode(), event->cost());
		break;

	case CUSTOM:
		switch(event->type()) {
		case LOCAL: {
				ParExeNode *n = findNode(inst, event->unit());
				ASSERT(n);
				removeLatency(n, event->cost());
			}
			break;
		case AFTER:
		case NOT_BEFORE:
			graph->remove(custom.get(event));
			break;
		}
		break;

	default:
		ASSERTP(0, _ << "unsupported event kind " << event->kind());
		break;
	}

}


/**
 * Lookup for the instruction i back from instruction f.
 * @param i	Looked instruction.
 * @param f	Instruction to look backward from.
 * @return	Corresponding parexgraph instruction (assertion raised if not found).
 */
ParExeInst *EdgeTimeBuilder::findInst(Inst *i, ParExeInst *f) {
	ParExeInst *last = nullptr;
	for(ParExeSequence::InstIterator xi(seq); xi() && *xi != f; xi++)
		if(xi->inst() == i)
			last = *xi;
	return last;
}


/**
 * Find in the given XG instruction the node corresponding to the given unit.
 * Fall through assertion failure if the unit node cannot be found.
 * @param i		Current XG instruction.
 * @param u		Looked unit.
 * @return		XG node corresponding to the XG instruction i and to the unit u.
 */
ParExeNode *EdgeTimeBuilder::findNode(ParExeInst *i, const hard::PipelineUnit *u) {
	for(ParExeInst::NodeIterator n(i); n(); n++)
		if(n->stage()->unit() == u)
			return *n;
	ASSERT(false);
	return nullptr;
}


/**
 * Find the node corresponding to the location pair (instruction, unit).
 * If the node cannot be found, an assertion failure is raised.
 * @param loc	Looked location.
 * @param from	Relative instruction to seach from.
 * @return		Corresponding XG node.
 */
ParExeNode *EdgeTimeBuilder::findNode(Pair<Inst *, const hard::PipelineUnit *> loc, ParExeInst *from) {
	ParExeInst *xi = findInst(loc.fst, from);
	if(xi == nullptr)
		return nullptr;
	else
		return findNode(xi, loc.snd);
}


/**
 * Add the latency to the node.
 * @param n	Node to add latency in.
 * @param l	Latency to add.
 */
void EdgeTimeBuilder::addLatency(ParExeNode *n, int l) {
	if(n->latency() == 1)
		n->setLatency(l);
	else
		n->setLatency(n->latency() + l);
}


/**
 * Remove the latency from the node.
 * @param n	Node to remove latency from.
 * @param l	Latency to remove.
 */
void EdgeTimeBuilder::removeLatency(ParExeNode *n, int l) {
	if(n->latency() == l)
		n->setLatency(1);
	else
		n->setLatency(n->latency() - l);
}


/**
 * Process the computed time and generate associated objective functions and constraints.
 */
void EdgeTimeBuilder::processTimes(const config_list_t& confs) {
	//applyStrictSplit(confs);
	//applyFloppySplit(confs);
	this->applyWeightedSplit(confs);
}


/**
 * Application of two time approach, LTS and HTS time.
 * To perform the split, an evaluation function is maximized according the two sets so that:
 * @li difference of time in LTS is the lowest,
 * @li difference of time between HTS and LTS is the biggest.
 * @li a feasible HTS is also a good point,
 */
void EdgeTimeBuilder::applyFloppySplit(const config_list_t& confs) {
	static const float	sep_factor = 1,			// TODO: put them in configuration
						span_factor = 1;

	// initialization
	int p = 1, best_p = -1;
	float best_cost = type_info<float>::min;
	int min_low = confs[0].time(), max_low = confs[0].time();		// , cnt_low = confs[0].count();
	int min_high = confs[1].time();									// , max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);

	// computation
	for(p = 1; p <= confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);
		min_high = confs[p - 1].time();
		max_low = confs[p].time();

		// select the best split
		bool feasible = set.isFeasible(events.length());
		float cost =
			sep_factor * (min_high - max_low)
			- span_factor * (max_low - min_low)
			+-int(feasible) * (min_high - max_low);
		if (isVerbose())
			log << "\t\t\t p = " << p << " -> " << cost << io::endl;
		if(cost > best_cost) {
			best_p = p;
			best_cost = cost;
		}

	}

	// consider all together
	if(best_p < 0) {
		genForOneCost(confs.top().time(), edge, all_events);
		return;
	}

	// look in the split
	ConfigSet hts;
	ot::time lts_time, hts_time;
	makeSplit(confs, best_p, hts, lts_time, hts_time);
	t::uint32 pos, neg, unu, com;
	hts.scan(pos, neg, unu, com, events.length());
	if(isVerbose())
		log << "\t\t\t\t"
			<< "pos = " << maskToString(pos, events.length())
			<< ", neg = " << maskToString(neg, events.length())
			<< ", unused = " << maskToString(unu, events.length())
			<< ", complex = " << maskToString(com, events.length())
			<< io::endl;

	// contribute
	contributeSplit(confs, pos, neg, 0, lts_time, hts_time);
}


/**
 * Performing split by computing an heuristic based on weights of BB
 * and events.
 * @param confs		List of configurations.
 */
void EdgeTimeBuilder::applyWeightedSplit(const config_list_t& confs) {

	// initialization
	int best_p = 0;
	ot::time best_cost = type_info<ot::time>::max;

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);

	// computation
	//for(int p = 1; p <= confs.length(); p++) {	// TODO -- why this error? Is it really an error?
	for(int p = 1; p < confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);

		// scan the set of values
		t::uint32 pos, neg, unu, com;
		set.scan(pos, neg, unu, com, events.length());

		// x^c_hts = sum{e in E_i /\ (\E c in HTS /\ e in c) /\ (\E c in HTS /\ e not in c)} w_e
		ot::time x_hts = 0;
		for(int i= 0; i < events.length(); i++)
			if(com & (1 << i))
				x_hts += events[i].fst->weight();

		// x^p_hts = max{e in E_i /\ (\A c in HTS -> e in c)} w_e
		for(int i= 0; i < events.length(); i++)
			if(pos & (1 << i))
				x_hts = max(x_hts, ot::time(events[i].fst->weight()));
		// x_hts = max(x^c_hts, x^p_hts)

		// cost = x_hts t_hts + (x_i - x_hts) t_lts
		int weight = WEIGHT(edge->target());
		if(x_hts > weight)
			x_hts = weight;
		ot::time cost = x_hts * confs.top().time() + (weight - x_hts) * confs[p - 1].time();
		if (logFor(LOG_BB))
			log << "\t\t\t\tHTS [" << p << " - " << (confs.length() - 1) << "], cost = " << cost << " (" << x_hts << "/" << weight << ")\n";

		// look for best cost
		if(cost < best_cost) {
			best_p = p;
			best_cost = cost;
		}
	}
	if (logFor(LOG_BB))
		log << "\t\t\t\tbest HTS [" << best_p << " - " << (confs.length() - 1) << "]\n";

	// look in the split
	ConfigSet hts;
	ot::time lts_time, hts_time;
	makeSplit(confs, best_p, hts, lts_time, hts_time);
	t::uint32 pos, neg, unu, com;
	hts.scan(pos, neg, unu, com, events.length());
	if(logFor(LOG_BB))
		log << "\t\t\t\t"
			<< "pos = " << maskToString(pos, events.length())
			<< ", neg = " << maskToString(neg, events.length())
			<< ", unused = " << maskToString(unu, events.length())
			<< ", complex = " << maskToString(com, events.length())
			<< io::endl;

	// contribute
	contributeSplit(confs, pos, neg, com, lts_time, hts_time);
}


/**
 * Application of two time approach, LTS and HTS time.
 * To perform the split, an evaluation function is maximized according the two sets so that:
 * @li the HTS is feasible,
 * @li difference of time in LTS is the lowest,
 * @li difference of time between HTS and LTS is the biggest.
 */
void EdgeTimeBuilder::applyStrictSplit(const config_list_t& confs) {
	static const float	sep_factor = 1,			// TODO: put them in configuration
						span_factor = 1;

	// initialization
	int p = 1, best_p = -1;
	float best_cost = type_info<float>::min;
	int min_low = confs[0].time(), max_low = confs[0].time();		//, cnt_low = confs[0].count();
	int min_high = confs[1].time();									//, max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);

	// computation
	for(p = 1; p < confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);
		min_high = confs[p - 1].time();
		max_low = confs[p].time();

		// select the best split
		float cost = sep_factor * (min_high - max_low) - span_factor * (max_low - min_low);
		if(cost > best_cost && set.isFeasible(events.length())) {
			best_p = p;
			best_cost = cost;
		}

	}

	// consider all together
	if(best_p < 0) {
		genForOneCost(confs.top().time(), edge, all_events);
		return;
	}

	// look in the split
	ConfigSet hts;
	ot::time lts_time, hts_time;
	makeSplit(confs, best_p, hts, lts_time, hts_time);
	t::uint32 pos, neg, unu, com;
	hts.scan(pos, neg, unu, com, events.length());
	if(isVerbose())
		log << "\t\t\t\t"
			<< "pos = " << maskToString(pos, events.length())
			<< ", neg = " << maskToString(neg, events.length())
			<< ", unused = " << maskToString(unu, events.length())
			<< ", complex = " << maskToString(com, events.length())
			<< io::endl;

	// too complex case
	if(com) {
		if(isVerbose())
			log << "\t\t\t\ttoo complex: " << io::hex(com) << io::endl;
		genForOneCost(hts_time, edge, all_events);
		return;
	}

	// contribute
	contributeSplit(confs, pos, neg, 0, lts_time, hts_time);
}


/**
 * Build the set after split.
 * @param confs		Current configuration.
 * @param p			Split position.
 * @param hts		HTS result set.
 * @param lts_time	LTS time.
 * @param hts_time	HTS time.
 */
void EdgeTimeBuilder::makeSplit(const config_list_t& confs, int p, ConfigSet& hts, ot::time& lts_time, ot::time& hts_time) {
	lts_time = confs[p - 1].time();
	hts_time = confs.top().time();
	hts = ConfigSet(hts_time);
	for(int i = p; i < confs.length(); i++)
		hts.add(confs[i]);
	if(logFor(LOG_BB)) {
		log << "\t\t\t\t" << "LTS time = " << lts_time << ", HTS time = " << hts_time << " for { ";
		bool fst = true;
		for(ConfigSet::Iter conf(hts); conf(); conf++) {
			if(fst)
				fst = false;
			else
				log << ", ";
			log << (*conf).toString(events.length());
		}
		log << " }\n";
	}
}

/**
 * Contribute to WCET estimation in split way, x_HTS and x_LTS,
 * with two sets of times.
 * @param confs		Time configuration.
 * @param p			Position of split.
 */
void EdgeTimeBuilder::contributeSplit(const config_list_t& confs, t::uint32 pos, t::uint32 neg, t::uint32 com, ot::time lts_time, ot::time hts_time) {

	// new HTS variable
	string hts_name;
	if(_explicit) {
		StringBuffer buf;
		buf << "e_";
		if(source)
			buf << source->index() << "_";
		buf << target->index() << "_" << target->cfg()->label() << "_hts";
		hts_name = buf.toString();
	}
	ilp::Var *x_hts = sys->newVar(hts_name);

	// wcet += time_lts x_edge + (time_hts - time_lts) x_hts
	ilp::Var *x_edge = ipet::VAR(edge);
	sys->addObjectFunction(lts_time, x_edge);
	sys->addObjectFunction(hts_time - lts_time, x_hts);
	if(record) {
		LTS_TIME(edge) = lts_time;
		HTS_CONFIG(edge).add(pair(hts_time - lts_time, x_hts));
	}

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
			get(events[i].fst)->contribute(make(events[i].fst, events[i].snd, true), x_hts);

		// else if e in neg_events then C^e_p += x_edge - x_hts / p = prefix if e in prefix, block
		else if(neg & (1 << i))
			get(events[i].fst)->contribute(make(events[i].fst, events[i].snd, false), x_hts);

		// else unprecise(C^e_p) = T / p = prefix if e in prefix, block
		else {
			get(events[i].fst)->contribute(make(events[i].fst, events[i].snd, true), 0);
			get(events[i].fst)->contribute(make(events[i].fst, events[i].snd, false), 0);
		}
	}

	// generate constant contribution
	contributeConst();

	// special contribution with com
	if(com) {

		// test that all contributes
		bool all = true;
		for(int i = 0; i < events.length(); i++)
			if((com & (1 << i)) && !events[i].fst->isEstimating(true)) {
				all = false;
				break;
			}

		// sum{e in com} x_e >= x_hts
		if(all) {
			static string msg = "complex constraint for times";
			ilp::Constraint *c = sys->newConstraint(msg, ilp::Constraint::GE, 0);
			for(int i = 0; i < events.length(); i++)
				if((com & (1 << i)) && events[i].fst->isEstimating(true))
					events[i].fst->estimate(c, true);
			c->addRight(1, x_hts);
		}
	}

}


/**
 * Generate contribution for constant events.
 */
void EdgeTimeBuilder::contributeConst(void) {

	// foreach e in always(e) do C^e_p += x_edge
	for(event_list_t::Iter event(all_events); event(); event++)
		switch((*event).fst->occurrence()) {
		case ALWAYS:	get((*event).fst)->contribute(make((*event).fst, (*event).snd, true), ipet::VAR(edge)); break;
		case NEVER:		get((*event).fst)->contribute(make((*event).fst, (*event).snd, false), ipet::VAR(edge)); break;
		default:		break;
		}
}


/**
 * This feature ensures that block cost has been computed according to the context
 * of edges. Basically, this means that both the objective function and the constraint
 * of events has been added to the ILP system.
 *
 * @p Configuration
 * @li @ref EVENT_THRESHOLD
 * @li @ref GRAPHS_OUTPUT_DIRECTORY
 * @li @ref ONLY_START
 * @li @ref PREDUMP
 * @li @ref RECORD_TIME
 *
 * @p Properties
 * @li @ref LTS_TIME
 * @li @ref HTS_OFFSET
 * @ingroup etime
 */
p::feature EDGE_TIME_FEATURE("otawa::etime::EDGE_TIME_FEATURE", new Maker<EdgeTimeBuilder>());

/**
 * Configuration property of EDGE_TIME_FEATURE, if true, enable the production
 * of LTS_TIME and HTS_OFFSET on the edge.
 * @ingroup etime
 */
p::id<bool> RECORD_TIME("otawa::etime::RECORD_TIME", false);

/**
 * Produced only if the RECORD_TIME configuration is set,
 * record for each edge the low-time-set maximum time in cycles.
 *
 * @par Hooks
 * @li @ref Edge
 * @ingroup etime
 */
p::id<ot::time> LTS_TIME("otawa::etime::LTS_TIME", -1);

/**
 * Produced only if the RECORD_TIME configuration is set,
 * record for each edge a pair(t, x) where (a) t the difference between maximum time
 * of low-time-set and the maximum time of high-time-set (in cycles)
 * (b) x is an ILP variable that represents the occurrences of this time.
 *
 * If an instruction sequence s with occurrence x_s has too many events, it will split in
 * subsequence and each subsequence will have its own HTS_CONFIG accumulated on the
 * sequence.
 *
 * @par Hooks
 * @li @ref Edge
 *
 * @ingroup etime
 */
p::id<Pair<ot::time, ilp::Var *> > HTS_CONFIG("otawa::etime::HTS_CONFIG", pair<ot::time, ilp::Var *>(0, nullptr));


/**
 * This property is used to configure the @ref EDGE_TIME_FEATURE and ask to dump the generated
 * execution graphs.
 * @ingroup etime
 */
p::id<bool> PREDUMP("otawa::etime::PREDUMP", false);


/**
 * This property is used to configure the @ref EDGE_TIME_FEATURE  and determine the maximum number of
 * events to consider to time a block. If a value of n is passed, at most 2^n times will be computed
 * and if a block gets a bigger number of events, it will be split.
 * @ingroup etime
 */
p::id<int> EVENT_THRESHOLD("otawa::etime::EVENT_THRESHOLD", 15);

} }	// otawa::etime
