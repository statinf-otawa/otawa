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


p::declare reg = p::init("otawa::etime::EdgeTimeBuilder", Version(1, 0, 0))
	.base(GraphBBTime<ParExeGraph>::reg)
	.maker<EdgeTimeBuilder>();

/**
 */
EdgeTimeBuilder::EdgeTimeBuilder(p::declare& r): GraphBBTime<ParExeGraph>(r) {
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
	avl::Set<Event *, EventComparator> source_events;
	if(source)
		for(Identifier<Event *>::Getter event(source, EVENT); event; event++)
			source_events.add(event);
	int source_size = source_events.count();
	avl::Set<Event *, EventComparator> target_events;
	for(Identifier<Event *>::Getter event(target, EVENT); event; event++)
		target_events.add(event);
	int total_size = target_events.count();

	// applying simple events (always, never)
	genstruct::Vector<Event *> events;
	genstruct::Vector<ParExeInst *> insts;
	ParExeSequence::InstIterator inst(seq);
	for(avl::Set<Event *, EventComparator>::Iterator event(source_events); event; event++) {
		while(inst->inst() != event->inst()) {
			inst++;
			ASSERT(inst);
		}
		switch(event->occurrence()) {
		case NEVER:		continue;
		case SOMETIMES:	events.add(*event); insts.add(*inst); break;
		case ALWAYS:	apply(event, inst); break;
		default:		ASSERT(0); break;
		}
	}
	for(avl::Set<Event *, EventComparator>::Iterator event(target_events); event; event++) {
		while(inst->inst() != event->inst()) {
			inst++;
			ASSERT(inst);
		}
		switch(event->occurrence()) {
		case NEVER:		continue;
		case SOMETIMES:	events.add(*event); break;
		case ALWAYS:	apply(event, inst); break;
		default:		ASSERT(0); break;
		}
	}

	// check number of events limit
	// TODO	fall back: analyze the block alone, split the block
	if(events.count() >= 32)
		throw ProcessorException(*this, _ << "too many events on edge " << edge);

	// simple trivial case
	if(events.isEmpty()) {
		ot::time cost = graph->analyze();
		ipet::TIME_DELTA(edge) = cost;
		return;
	}

	// compute all cases
	t::uint32 prev = 0;
	for(t::uint32 mask = 0; mask < 1 << events.count(); mask++) {

		// adjust the graph
		for(int i = 0; i < events.count(); i++)
			if((prev & (1 << i)) != (mask & (1 << i))) {
				if(mask & (1 << i))
					apply(events[i], insts[i]);
				else
					rollback(events[i], insts[i]);
			}

		// compute and store the new value
		ot::time cost = graph->analyze();

	}

	// build worst and best sets

	// add to the objective function

	// add to the constraints

	// apply and filter the events
	/*const Events& events = EVENTS(edge);
	genstruct::Vector<Event *> act_events;
	ParExeSequence::InstIterator exe_inst(seq);
	genstruct::Vector<Switch *> switches;
	for(int i = 0; i < events.count(); i++)
		if(events[i]->occurrence() != Event::NEVER) {
			Switch *sw;
			switch(events[i]->delay()) {
			case Event::TO_STAGE:
				sw = makeStageEvent(exe_inst, static_cast<StageEvent *>(events[i]));
				break;
			case Event::INTER_STAGE:
				sw = makeInterEvent(exe_inst, static_cast<InterEvent *>(events[i]));
				break;
			}
			if(events[i]->occurrence() == Event::ALWAYS) {
				sw->apply();
				delete sw;
			}
			else {
				act_events.add(events[i]);
				switches.add(sw);
			}
		}*/


	// compute the times
	/*if(logFor(Processor::LOG_BLOCK)) {
		log << "\t\t\t\tevents = ";
		for(int i = switches.length() - 1; i >= 0; i--)
			log << switches[i] << ' ';
		log << io::endl;
	}
	genstruct::Vector<ot::time> times;
	int prev = 0;
	for(t::uint32 i = 0; i < (1 << act_events.length()); prev = i, i++) {

		// change activation
		for(int j = act_events.length() - 1; j >= 0; j--) {
			t::uint32 mask = 1 << j;
			if((prev & mask) != (i & mask)) {
				if(mask & i)
					switches[j]->apply();
				else
					switches[j]->rollback();
			}
		}

		// debug
		//for(int j = 0; j < switches.length(); j++)
		//	log << switches[j];

		// perform the computation
		ot::time cost = graph.analyze();
		times.add(cost);
		if(logFor(Processor::LOG_BLOCK))
			log << "\t\t\t\ttime(" << io::pad('0', io::right(io::width(switches.length(), io::bin(i)))) << ") = " << cost << io::endl;
	}*/

	// build the time set
	//TIME_SET(edge) = new TimeSet(act_events.detach(), times.detach());

	// cleanup
	//for(int i = 0; i < switches.length(); i++)
	//	delete switches[i];
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

} }	// otawa::etime
