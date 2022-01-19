/*
 *	StandardGenerator class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <elm/data/quicksort.h>
#include <otawa/ipet.h>
#include <otawa/etime/StandardILPGenerator.h>

namespace otawa { namespace etime {

/**
 * @class ILPGenerator
 *
 * This class is a part of time generation by the etime module.
 * It is in charge of generating the objective function (for IPET approach)
 * and of the constraints that links the occurrence of BB times and the
 * corresponding events.
 *
 * This class may use configuration of @ref AbstractTimeBuilder:
 * * explicit -- to assign explicit names to generated ILP variables,
 * * recording -- to add recording of times as properties (otawa::etime::LTS_TIME,
 * and otawa::etime::HTS_CONFIG).
 *
 * @ingroup etime
 */

/**
 */
ILPGenerator::ILPGenerator(Monitor& mon): Monitor(mon), _atb(nullptr) {
}


/**
 */
ILPGenerator::~ILPGenerator(void) {
}



/**
 * Get the current ILP system.
 * @return	Current ILP system.
 */
ilp::System *ILPGenerator::system(void) const {
	return _atb->_sys;
}


/**
 * Test if explicit variable naming is requiring.
 * @return	True if it is required, false else.
 */
bool ILPGenerator::isExplicit(void) const {
	return _atb->_explicit;
}

/**
 * Test if the time needs to be recorded.
 * @return	True if times needs to be recorded, false else.
 */
bool ILPGenerator::isRecording(void) const {
	return _atb->_record;
}

/**
 * Called by the AbstractTimeBuilder to let the component
 * configure itself.
 * @param props		Configuration to read from.
 */
void ILPGenerator::configure(const PropList& props) {
}

/**
 * @fn ilp::System *ILPGenerator::system(void) const;
 * Get the current ILP system.
 * @return	Current ILP system.
 */

/**
 * @fn bool ILPGenerator::isExplicit(void) const;
 * Test if the explicit option is set.
 * @return	Value of explicit option.
 */

/**
 * @fn bool ILPGenerator::isRecording(void) const;
 * Test if the recording option is.
 * @return	Value of recording option.
 */

/**
 * @fn void ILPGenerator::process(WorkSpace *ws);
 * This function is called by AbstractTimeBuilder to let the ILPGenerator
 * drives the ILP building for the given workspace.
 * @param ws	Workspace to work with.
 */

/**
 * @fn void ILPGenerator::contributeBase(ot::time time);
 * Function called by the solver to record a basic time for the current
 * code sequence.
 *
 * @warning This is the first function called after an execution graph
 * resolution.
 *
 * @param time	Basic time.
 */

/**
 * @fn void ILPGenerator::contributeTime(ot::time time);
 * Function called by the solver to record an alternative time for
 * the current code sequence.
 *
 * @warning This call only happens after a former call to contributeBase().
 *
 * @param time	Alternative time.
 */

/**
 * @fn void ILPGenerator::contributePositive(EventCase event, bool prec);
 * This call applies to the last contributeTime() and record that the time
 * occurrence is bound by the given event activation.
 * @param event	Bounding event.
 * @param prec	If true, the bound is precise else it is an overestimation.
 */

/**
 * @fn void ILPGenerator::contributeNegative(EventCase event, bool prec);
 * This call applies to the last contributeTime() and record that the time
 * occurrence is bound by the given event non-activation.
 * @param event	Bounding event.
 * @param prec	If true, the bound is precise else it is an overestimation.
 */

/**
 * Convenient function used to build the execution graph for the given
 * sequence.
 * @param seq	Sequence to build graph for.
 * @return		Built execution graph.
 */
ParExeGraph *ILPGenerator::build(ParExeSequence *seq) {
	return builder()->builder()->build(seq);
}

/**
 * Convenient function used to solve the graph.
 * During the resolution, the solver may call the functions:
 * * contributeBase()
 * * contributeTime()
 * * contributePositive()
 * * contributeNegative()
 * @param g			Graph to solve.
 * @param events	Ordered list of events (according to instruction address  and postion).
 */
void ILPGenerator::solve(const PropList *entity, ParExeGraph *g, const Vector<EventCase>& events) {
	builder()->solver()->compute(entity, g, events);
}

/**
 * Collect the events stored in the given property list.
 * @param events	Where to store the found events.
 * @param props		Property list to look in.
 * @param part		Part of the sequence the events are contained.
 * @param id		Identifier of the events to collect.
 */
void ILPGenerator::collectEvents(Vector<EventCase>& events, PropList *props, part_t part, p::id<Event *>& id) {
	for(auto e: id.all(props))
		events.add(EventCase(e, part));
}

/**
 * Convenient function used to collect events of prologue block.
 * @param events
 * @param v	Block to look in.
 */
void ILPGenerator::collectPrologue(Vector<EventCase>& events, Block *v) {
	collectEvents(events, v, IN_PREFIX, EVENT);
}

/**
 * Convenient function used to collect events of the block itself.
 * @param events
 * @param v	Block to look in.
 */
void ILPGenerator::collectBlock(Vector<EventCase>& events, Block *v) {
	collectEvents(events, v, IN_BLOCK, EVENT);
}

/**
 * Convenient function used to collect events in an edge.
 * @param events
 * @param v	Block to look in.
 */
void ILPGenerator::collectEdge(Vector<EventCase>& events, Edge *e) {
	collectEvents(events, e, IN_PREFIX, PREFIX_EVENT);
	collectEvents(events, e, IN_BLOCK, EVENT);
}

/**
 * Sort the events according to their position in the sequence.
 */
void ILPGenerator::sortEvents(Vector<EventCase>& events) {

	// sort events
	class EventCaseComparator {
	public:
		inline int doCompare(const EventCase& c1, const EventCase& c2) const{
			if(c1.part() != c2.part()){
				return c1.part() - c2.part();
			}
			else{
				if (c1.event()->inst()->address() != c2.event()->inst()->address())
					return c1.event()->inst()->address().compare(c2.event()->inst()->address());
				else
					//return reinterpret_cast<intptr_t>(c2.event()->unit()) - reinterpret_cast<intptr_t>(c1.event()->unit());
					return -Comparator<const hard::PipelineUnit *>::compare(c2.event()->unit(),c1.event()->unit());
			}
		}
	};
	elm::quicksort(events, EventCaseComparator());

	// index the dynamic events
	int di = 0;
	for(int i = 0; i < events.length(); i++)
		if(events[i].event()->occurrence() == etime::SOMETIMES) {
			events[i].setIndex(di);
			di++;
		}

}

/**
 * Count the number of dynamic events in the given event collection.
 * @param events	Event collection to count in.
 * @return			Number of dynamic events in the event collection.
 */
int ILPGenerator::countDyn(const Vector<EventCase>& events) {
	int cnt = 0;
	for(auto e: events)
		if(e->occurrence() == SOMETIMES)
			cnt++;
	return cnt;
}



/**
 * @class StandardILPGenerator
 * The default implementation of ILPGenerator.
 * Apply the LTS/HTS method resulting in two times for each BB.
 * @ingroup etime
 */

/**
 */
StandardILPGenerator::EventCollector::EventCollector(Event *event): imprec(0), evt(event) {
}

/**
 * Add an imprecise contribution.
 * @param c		Event case.
 */
void StandardILPGenerator::EventCollector::boundImprecise(EventCase c) {
	if(c.part() == IN_PREFIX)
		imprec |= (1 << PREFIX_OFF) | (1 << PREFIX_ON);
	else
		imprec |= (1 << BLOCK_OFF) | (1 << BLOCK_ON);
}

/**
 * Add a code sequence variable to an activation of the event.
 * @param c		Event case.
 * @param x		Occurrence count variable.
 * @param prec 	True if the bound is precise.
 */
void StandardILPGenerator::EventCollector::boundPositive(EventCase c, ilp::Var *x, bool prec) {
	case_t cc;
	if(c.part() == IN_PREFIX)
		cc = PREFIX_ON;
	else
		cc = BLOCK_ON;
	if(!prec)
		imprec |= 1 << cc;
	vars[cc].add(x);
}

/**
 * Add a code sequence variable to an non-activation of the event.
 * @param c		Event case.
 * @param x		Occurrence count variable.
 * @param prec 	True if the bound is precise.
 */
void StandardILPGenerator::EventCollector::boundNegative(EventCase c, ilp::Var *x, bool prec) {
	case_t cc;
	if(c.part() == IN_PREFIX)
		cc = PREFIX_OFF;
	else
		cc = BLOCK_OFF;
	if(!prec)
		imprec |= 1 << cc;
	vars[cc].add(x);
}

/**
 * Make the variable and constraint for the current event.
 * According to the variable list and the provided overestimation,
 * may generate zero, one or several constraints.
 * @param sys	System to create constraints in.
 */
void StandardILPGenerator::EventCollector::make(ilp::System *sys) {
	static string label = "time occurrence bounding by events";
	for(int c = 0; c < SIZE; ++c) {
		if(vars[c] && evt->isEstimating(isOn(case_t(c)))) {
			ilp::Constraint *cons = sys->newConstraint(
				label,
				/*(imprec & (1 << c)) ?*/ ilp::Constraint::GE /*: ilp::Constraint::EQ*/);
			evt->estimate(cons, isOn(case_t(c)));
			for(List<ilp::Var *>::Iter v(vars[c]); v(); v++)
				cons->addRight(1, *v);
		}
	}
}


/**
 */
StandardILPGenerator::StandardILPGenerator(Monitor& mon):
	ILPGenerator(mon),
	_edge(nullptr),
	_t_lts(-1),
	_x_e(nullptr),
	_x_hts(nullptr),
	_t_lts_set(false),
	_eth(0),
	_ilp_var_count(0)
{ }


///
void StandardILPGenerator::configure(const PropList& props) {
	ILPGenerator::configure(props);
	_eth = etime::EVENT_THRESHOLD(props);
}


/**
 */
void StandardILPGenerator::process(WorkSpace *ws) {

	// process each edge
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	for(auto g: *coll) {
		if(logFor(LOG_FUN))
			log << "\tCFG " << g << io::endl;
		for(auto v: *g)
			if(v->isBasic())
				for(auto e: v->inEdges()) {
					if(logFor(LOG_BLOCK))
						log << "\t\t" << e << io::endl;
					process(e);
				}
	}
	if(logFor(LOG_PROC))
		log << "\t\t\t\t ILP VARS COUNT = "<< _ilp_var_count << io::endl;

	// generate the bounding constraints of the events
	for(auto coll: colls) {
		coll->make(system());
		delete coll;
	}
	colls.clear();
}

/**
 */
void StandardILPGenerator::contributeBase(ot::time time) {
	ASSERTP(!_t_lts_set, "several contributeBase() performed");
	_t_lts_set = true;
	_t_lts = time;
	_ilp_var_count++;

	// logging
	if(logFor(LOG_BB))
		log << "\t\t\t\tcost = " << time << io::endl;

	// add to the objective function
	system()->addObjectFunction(time, _x_e);
	if(isRecording())
		LTS_TIME(_edge) = time;
}

/**
 */
void StandardILPGenerator::contributeTime(ot::time t) {
	//No need to call contributeBase first
	ASSERTP(_t_lts_set, "perform contributeBase() first");
	//
	_ilp_var_count++;

	// new variable
	string hts_name;
	if(isExplicit()) {
		StringBuffer buf;
		buf << "e_";
		buf << _edge->source()->index() << "_";
		buf << _edge->sink()->index() << "_"
			<< _edge->sink()->cfg()->label() << "_hts";
		hts_name = buf.toString();
	}
	//_x_hTS is used as 'current' i.e. last variable created by contributeTime
	_x_hts = system()->newVar(hts_name);

	// wcet += time_lts x_edge + (time_hts - time_lts) x_hts
	system()->addObjectFunction(t - _t_lts , _x_hts);
	if(isRecording())
		HTS_CONFIG(_edge).add(pair(t - _t_lts, _x_hts));

	// 0 <= x_hts <= x_edge
	ilp::Constraint *cons = system()->newConstraint("0 <= x", ilp::Constraint::LE);
	cons->addRight(1, _x_hts);
	_partitionVars.push(_x_hts);
	//cons = system()->newConstraint("x_hts <= x_edge", ilp::Constraint::LE);
	//cons->addLeft(1, _x_hts);
	//cons->addRight(1, _x_e);
}

/**
 */
void StandardILPGenerator::contributePositive(EventCase event, bool prec) {
	ASSERTP(_x_hts != nullptr, "call contributeTime() fist");
	get(event.event())->boundPositive(event, _x_hts, prec);
	_done.set(event.index());
}

/**
 */
void StandardILPGenerator::contributeNegative(EventCase event, bool prec) {
	ASSERTP(_x_hts != nullptr, "call contributeTime() fist");
	get(event.event())->boundNegative(event, _x_hts, prec);
	_done.set(event.index());
}

/**
 */
void StandardILPGenerator::prepare(Edge *e, const Vector<EventCase>& events, int dyn_cnt) {
	_edge = e;
	_t_lts = -1;
	_t_lts_set = false;
	_x_e = ipet::VAR(e);
	_x_hts = nullptr;
	_partitionVars.clear();
	
	if(dyn_cnt > 0)
		_done.resize(dyn_cnt);
	_done.clear();
}

/**
 */
void StandardILPGenerator::finish(const Vector<EventCase>& events) {

	// bound for non-variable events
	// foreach e in always(e) do C^e_p += x_edge
	for(auto e: events)
		switch(e->occurrence()) {
		case ALWAYS:	get(e.event())->boundPositive(e, _x_e, true); break;
		case NEVER:		get(e.event())->boundNegative(e, _x_e, true); break;
		default:		break;
		}

	// all non-processed dynamic causes imprecision
	for(auto e: events)
		if(e->occurrence() == SOMETIMES
		&& !_done.bit(e.index()))
			get(e.event())->boundImprecise(e);


	if (!_partitionVars.isEmpty()){
	ilp::Constraint* cons = system()->newConstraint("sum(x) = x_edge", ilp::Constraint::LE);
	cons->addRight(1, _x_e);
	for (auto v: _partitionVars){
		cons->addLeft(1, v);
	}
	}
}

///
void StandardILPGenerator::process(Edge *e, ParExeSequence *seq, Vector<EventCase>& events, int dyn_cnt) {

	// log events
	if(logFor(LOG_BB))
		for(auto e: events)
			log << "\t\t\t" << e << io::endl;

	// numbers the dynamic events
	int cnt = 0;
	for(int i = 0; i < events.count(); i++)
		if(events[i].event()->occurrence() == SOMETIMES)
			events[i].setIndex(cnt++);

	// build the graph
	prepare(e, events, dyn_cnt);
	ParExeGraph *g = build(seq);
	solve(e, g, events);
	finish(events);

	// clean up
	delete g;
}

///
void StandardILPGenerator::process(Edge *e) {
	BasicBlock *w = nullptr;
	if(e->source()->isBasic())
		w = e->source()->toBasic();
	BasicBlock *v = e->sink()->toBasic();

	// collect events
	Vector<EventCase> events;
	if(w != nullptr)
		collectPrologue(events, w);
	collectEdge(events, e);
	collectBlock(events, v);
	sortEvents(events);
	int dyn_cnt = countDyn(events);

	// build the sequence (if event threshold not reached
	if(dyn_cnt <= _eth) {
		ParExeSequence *seq = new ParExeSequence();
		int index = 0;
		if(w)
			for(auto i: *w)
				seq->addLast(new ParExeInst(i, w, PROLOGUE, index++));
		for(auto i: *v)
			seq->addLast(new ParExeInst(i, v, otawa::BLOCK, index++));
		process(e, seq, events, dyn_cnt);
		delete seq;
		return;
	}

	// try to remove prolog
	events.clear();
	collectBlock(events, v);
	sortEvents(events);
	dyn_cnt = countDyn(events);
	if(dyn_cnt <= _eth) {
		ParExeSequence *seq = new ParExeSequence();
		int index = 0;
		for(auto i: *v)
			seq->addLast(new ParExeInst(i, v, otawa::BLOCK, index++));
		process(e, seq, events, dyn_cnt);
		delete seq;
		return;
	}

	// starting split of the block
	if(logFor(LOG_BB))
		log << "\t\t\ttoo many dynamic events (" << dyn_cnt << "): split required\n";
	int ei = 0;
	auto ii = v->insts();
	while(ei < events.length()) {
		Vector<EventCase> split_events;

		// collect split events
		dyn_cnt = 0;
		Inst *faulty = nullptr;
		while(ei < events.length()) {
			split_events.add(events[ei]);
			if(events[ei].isDynamic())
				dyn_cnt++;
			if(dyn_cnt > _eth) {
				faulty = events[ei].inst();
				break;
			}
			ei++;
		}

		// roll-back events of the faulty instruction
		if(faulty != nullptr){
			while(events[ei].inst() == faulty) {
				if(events[ei].isDynamic())
					dyn_cnt++;
				ei--;
				split_events.removeLast();
			}
			ei++;
		}


		// compute the time
		ParExeSequence *seq = new ParExeSequence();
		int index = 0;
		while(ii() && (ei >= events.length() || *ii != events[ei].event()->inst())) {
			seq->addLast(new ParExeInst(*ii, v, otawa::BLOCK, index++));
			ii++;
		}
		if(logFor(LOG_BB))
			log << "\t\t\tcomputing for " << seq->first()->inst()->address()
				<< " to " << seq->last()->inst()->address() << io::endl;
		process(e, seq, split_events, dyn_cnt);
		delete seq;
	}
}

/**
 * Get the collector for the given event.
 * If it doesn't exist, create it.
 * @param event		Concerned event.
 * @return			Matching collector (never null).
 */
StandardILPGenerator::EventCollector *StandardILPGenerator::get(Event *event) {
	EventCollector *coll = colls.get(event, nullptr);
	if(!coll) {
		coll = new EventCollector(event);
		colls.put(event, coll);
	}
	return coll;
}

/**
 * Build the default ILP generator instance.
 */
ILPGenerator *ILPGenerator::make(Monitor& mon) {
	return new StandardILPGenerator(mon);
}

} }		// otawa::etime

