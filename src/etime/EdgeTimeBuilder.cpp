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
#include <elm/genstruct/quicksort.h>
#include <otawa/etime/Config.h>
#include <otawa/etime/EventCollector.h>

namespace otawa { namespace etime {

Identifier<bool> PREDUMP("otawa::etime::PREDUMP", false);


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
string Config::toString(int n) {
	StringBuffer buf;
	for(int i = 0; i < n; i++)
		buf << (bit(i) ? "+" : "-");
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
			genstruct::Vector<t::uint32> nconf, pconf;
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
		buf << ((mask & (1 << i)) ? "!" : "_");
	return buf.toString();
}


/**
 * @class EventCollector
 * Collects variables linking events with blocks in ILP.
 */


/**
 * @enum case_t;
 * Case for the variable between place (prefix/block) and activation of the event (on/off).
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
	//cerr << "DEBUG: making constraint for " << evt->inst()->address() << "\t" << evt->name() << "(type = " << int(evt->type()) << ")\n";
	int s = PREFIX_OFF ;
	if(evt->type() == EDGE)
		s = BLOCK_OFF;
	for(int c = s; c < SIZE; ++c) {
		if(vars[c] && evt->isEstimating(isOn(case_t(c)))) {
			ilp::Constraint *cons = sys->newConstraint(
				/*"event constraint"*/evt->name() ,
				/*(imprec & (1 << c)) ?*/ ilp::Constraint::GE /*: ilp::Constraint::EQ*/);
			evt->estimate(cons, isOn(case_t(c)));
			for(genstruct::SLList<ilp::Var *>::Iterator v(vars[c]); v; v++)
				cons->addRight(1, *v);
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
	if(e->type() == EDGE)
		return on ? EventCollector::BLOCK_ON : EventCollector::BLOCK_OFF;
	else
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
	.require(WEIGHT_FEATURE)
	.require(EVENTS_FEATURE)
	.provide(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(EDGE_TIME_FEATURE);


/**
 */
EdgeTimeBuilder::EdgeTimeBuilder(p::declare& r)
: GraphBBTime<ParExeGraph>(r), graph(0), source(0), target(0), seq(0) {
}


void EdgeTimeBuilder::configure(const PropList& props) {
	GraphBBTime<ParExeGraph>::configure(props);
	_explicit = ipet::EXPLICIT(props);
	predump = PREDUMP(props);
}


/**
 */
void EdgeTimeBuilder::setup(WorkSpace *ws) {
	sys = ipet::SYSTEM(ws);
}


/**
 */
void EdgeTimeBuilder::cleanup(WorkSpace *ws) {
	for(genstruct::HashTable<Event *, EventCollector *>::Iterator coll(colls); coll; coll++) {
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
	for(BasicBlock::InIterator in(bb); in; in++) {
		edge = in;
		processEdge(ws, cfg);
	}
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
void EdgeTimeBuilder::processEdge(WorkSpace *ws, CFG *cfg) {
	if(logFor(Processor::LOG_BLOCK))
		log << "\t\t\t" << edge << io::endl;

	// initialize the sequence
	bnode = 0;
	int index = 0;
	seq = new ParExeSequence();

	// compute source
	source = edge->source();
	if(source->isEntry()) {
		if(CALLED_BY(source->cfg()).exists())
			source = CALLED_BY(source->cfg())->source();
		else
			source = 0;
	}

	// compute target
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
	graph = make(seq);
	ASSERTP(graph->firstNode(), "no first node found: empty execution graph");

	// collect and sort events
	all_events.clear();
	if(source)
		sortEvents(all_events, source, IN_PREFIX);
	sortEvents(all_events, target, IN_BLOCK, edge);
	if(logFor(LOG_BB))
		for(genstruct::Vector<event_t>::Iterator e(all_events); e; e++)
			cerr << "\t\t\t\t" << (*e).fst->inst()->address() << " -> "
				 << (*e).fst->name() << " (" << (*e).fst->detail() << ") in "
				 << (*e).snd << io::endl;

	// applying static events (always, never)
	events.clear();
	genstruct::Vector<ParExeInst *> insts;
	event_list_t always_events;
	ParExeSequence::InstIterator inst(seq);
	for(event_list_t::Iterator event(all_events); event; event++) {
		Event *evt = (*event).fst;

		// find the instruction
		while((*event).snd != IN_PREFIX && inst->codePart() != otawa::BODY) {
			inst++;
			ASSERT(inst);
		}
		while(inst->inst() != evt->inst()) {
			inst++;
			ASSERT(inst);
		}

		// apply the event
		switch(evt->occurrence()) {
		case NEVER:			continue;
		case SOMETIMES:		events.add(*event); insts.add(*inst); break;
		case ALWAYS:		apply(evt, inst); always_events.add(*event); break;
		default:			ASSERT(0); break;
		}
	}

	// check number of events limit
	// TODO	fall back: analyze the block alone, split the block
	if(events.count() >= 32)
		throw ProcessorException(*this, _ << "too many events on edge " << edge);
	if(logFor(LOG_BB)) {
		log << "\t\t\t\tdynamic events = " << events.count() << io::endl;
		for(int i = 0; i < events.count(); i++)
			log << "\t\t\t\t" << events[i].fst->inst()->address() << "\t" << events[i].fst->name()
				<< " " << events[i].snd << io::endl;
	}

	// simple trivial case
	if(events.isEmpty()) {
		ot::time cost = graph->analyze();
		this->genForOneCost(cost, edge, events);
		delete graph;
		return;
	}

	// compute all cases
	t::uint32 prev = 0;
	genstruct::Vector<ConfigSet> confs;
	for(t::uint32 mask = 0; mask < 1 << events.count(); mask++) {

		// adjust the graph
		for(int i = 0; i < events.count(); i++) {
			if((prev & (1 << i)) != (mask & (1 << i))) {
				if(mask & (1 << i))
					apply(events[i].fst, insts[i]);
				else
					rollback(events[i].fst, insts[i]);
			}
		}
		prev = mask;

		// predump implementation
		if(_do_output_graphs && predump)
			outputGraph(graph, 666, 666, 666, _ << source << " -> " << target);

		// compute and store the new value
		ot::time cost = graph->analyze();

		// dump it if needed
		if(_do_output_graphs)
			outputGraph(graph, target->number(), source->number(), mask, _ << source << " -> " << target << " (cost = " << cost << ")");

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
			confs[confs.length() - 1].add(Config(mask));
		}
	}
	if(isVerbose())
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

	// generate constant contrubtion
	contributeConst();

	// generate variable contribution
	for(event_list_t::Iterator event(events); event; event++)
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
	for(Identifier<Event *>::Getter event(bb, EVENT); event; event++)
		set.add(pair(*event, place));
	if(edge)
		for(Identifier<Event *>::Getter event(edge, EVENT); event; event++)
			set.add(pair(*event, IN_EDGE));
	for(set_t::Iterator e(set); e; e++)
		events.push(*e);
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
	int min_low = confs[0].time(), max_low = confs[0].time(), cnt_low = confs[0].count();
	int min_high = confs[1].time(), max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);
	//cerr << "DEBUG: init = " << set.isFeasible(events.length()) << io::endl;

	// computation
	for(p = 1; p < confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);
		min_high = confs[p - 1].time();
		max_low = confs[p].time();

		// select the best split
		float cost = sep_factor * (min_high - max_low) - span_factor * (max_low - min_low);
		//log << "DEBUG: p = " << p << ", cost = " << cost << ", Ml = " << max_low << ", mh = " << min_high << ", ml = " << min_low << ", f = " << set.isFeasible(events.length()) << io::endl;
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
 * Get the branch node resolving a branch prediction.
 * @return	Node of resolution.
 */
ParExeNode *EdgeTimeBuilder::getBranchNode(void) {
	ASSERT(source);
	if(!bnode) {
		Inst *binst = source->controlInst();
		for(ParExeSequence::Iterator pinst(*seq); pinst; pinst++)
			if(pinst->inst() == binst) {
				for(ParExeInst::NodeIterator node(*pinst); node; node++)
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
	//cerr << "DEBUG: apply " << event->name() << io::endl;

	switch(event->kind()) {

	case FETCH:
		inst->fetchNode()->setLatency(inst->fetchNode()->latency() + event->cost());
		break;

	case MEM:
		for(ParExeInst::NodeIterator node(inst); node; node++)
			if(node->stage()->unit()->isMem()) {
				node->setLatency(node->latency() + event->cost() - 1);
				break;
			}
		break;

	case BRANCH:
		bedge =  new ParExeEdge(getBranchNode(), inst->fetchNode(), ParExeEdge::SOLID, 0, pred_msg);
		bedge->setLatency(event->cost());
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
	//cerr << "DEBUG: rollback " << event->name() << io::endl;

	switch(event->kind()) {

	case FETCH:
		inst->fetchNode()->setLatency(inst->fetchNode()->latency() - event->cost());
		break;

	case MEM:
		for(ParExeInst::NodeIterator node(inst); node; node++)
			if(node->stage()->unit()->isMem()) {
				node->setLatency(node->latency() - event->cost() + 1);
				break;
			}
		break;

	case BRANCH:
		ASSERT(bedge);
		cerr << "DEBUG: rollbacking " << (void *)bedge << io::endl;
		graph->remove(bedge);
		bedge = 0;
		break;

	default:
		ASSERTP(0, _ << "unsupported event kind " << event->kind());
		break;
	}

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
	int min_low = confs[0].time(), max_low = confs[0].time(), cnt_low = confs[0].count();
	int min_high = confs[1].time(), max_high = confs[confs.length() - 1].time();
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
		bool feasible = set.isFeasible(events.length());
		float cost =
			sep_factor * (min_high - max_low)
			- span_factor * (max_low - min_low)
			+-int(feasible) * (min_high - max_low);
		cerr << "\t\t\t p = " << p << " -> " << cost << io::endl;
		//log << "DEBUG: p = " << p << ", cost = " << cost << ", Ml = " << max_low << ", mh = " << min_high << ", ml = " << min_low << ", f = " << set.isFeasible(events.length()) << io::endl;
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
		cerr << "\t\t\t p = " << p << ", cost = " << cost << " (" << x_hts << "/" << weight << ")\n";

		// look for best cost
		if(cost < best_cost) {
			best_p = p;
			best_cost = cost;
		}
	}
	cerr << "\t\t\tbest_p = " << best_p << io::endl;

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
	int min_low = confs[0].time(), max_low = confs[0].time(), cnt_low = confs[0].count();
	int min_high = confs[1].time(), max_high = confs[confs.length() - 1].time();
	int cnt_high = 0;
	for(int i = 1; i < confs.length(); i++)
		cnt_high += confs[i].count();

	// set of configurations
	ConfigSet set;
	for(int i = confs.length() - 1; i >= 0; i--)
		set.push(confs[i]);
	//cerr << "DEBUG: init = " << set.isFeasible(events.length()) << io::endl;

	// computation
	for(p = 1; p < confs.length(); p++) {

		// update set and values
		set.pop(confs[p - 1]);
		min_high = confs[p - 1].time();
		max_low = confs[p].time();

		// select the best split
		float cost = sep_factor * (min_high - max_low) - span_factor * (max_low - min_low);
		//log << "DEBUG: p = " << p << ", cost = " << cost << ", Ml = " << max_low << ", mh = " << min_high << ", ml = " << min_low << ", f = " << set.isFeasible(events.length()) << io::endl;
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
		// sum{e in com} x_e >= x_hts
		static string msg = "complex constraint for times";
		ilp::Constraint *c = sys->newConstraint(msg, ilp::Constraint::GE, 0);
		for(int i = 0; i < events.length(); i++)
			if(com & (1 << i))
				events[i].fst->estimate(c, true);
		c->addRight(1, x_hts);
	}
}


/**
 * Generate contribution for constant events.
 */
void EdgeTimeBuilder::contributeConst(void) {

	// foreach e in always(e) do C^e_p += x_edge
	for(event_list_t::Iterator event(all_events); event; event++)
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
 */
p::feature EDGE_TIME_FEATURE("otawa::etime::EDGE_TIME_FEATURE", new Maker<EdgeTimeBuilder>());

} }	// otawa::etime
