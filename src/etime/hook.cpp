/*
 *	etime plugin hook
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

#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/etime/StandardEventBuilder.h>

namespace otawa { namespace etime {


/**
 * @defgroup etime	Event Time Module
 * This module aims to make more generic how events (cache hits/misses, branch prediction, etc)
 * are processed to generate block time.
 *
 * The basic idea is that each analysis contributes to the WCET by defining the events
 * (changes of the execution time) applying to instructions or a blocks. As a result, these events
 * are hooked to the basic block they apply to and defines (a) which instruction is concerned,
 * (b) which part of the instruction execution they apply to (memory access, stage, functional unit, etc)
 * and (c) how they contribute to the time.
 *
 * In addition, an event may c contribute to the constraints of the ILP system and may be asked
 * to provide an overestimation or an underestimation of their occurrence according to their
 * contribution (worst case time or best case time) to the WCET.
 *
 * From the events, several policies may be applied to compute the times, to select
 * the granularity and the precision of the computed time or to choose a way to split
 * the CFG into blocks.
 *
 * The execution times are computed based on a= "time unit". An execution unit is made
 * of a block, for which time is computed, and a prefix (made of 0 or several other blocks).
 * The prefix allows to more precisely take into account the overlap effect of sequential
 * execution of blocks. In the current version, we only support prefixes of 1 block but this
 * may be extended in the next versions of this plugin.
 *
 * The time unit are stored and built for each basic block by examining the predecessors
 * and counting the occurrences of this time using the corresponding edge. Because of the
 * CFG structure of OTAWA, the following rules are applied to build the prefixes of a basic
 * block v for a predecessor w:
 * @li if w is a basic block, the time unit has only one prefix block w on edge (w, v);
 * @li if w is the CFG entry and the CFG is the task entry, no prefix is considered;
 * @li if w is the CFG entry and the CFG may have several calls: for each call u through
 *     t synthetic block and u is a basic block, there is on prefix u on edge (u, t);
 *     if u is not a basic block (unfrequent but may happen to transformations of CFG),
 *     we consider there is a prefix on edge (u, t) but no prefix block;
 * @li if w is a synthetic block calling function f with only one last basic block u, the
 *     prefix is u and the edge (w, v);
 * @li if w is a synthetic block calling function f with several last basic blocks u, the
 *     prefixes is u with edges (u, exit);
 * @li else there is no prefix along edge (w, v).
 *
 * The structure of the CFG and the possible transformations of it does not allow to always
 * get (a) a right prefix and (b) a count of this prefix. Yet, as these cases are so infrequent
 * and as they be replaced by an overestimation of the actual time, they do not represent
 * an issue in the WCET computation.
 *
 * When building the set of time unit, one has to keep in mind that the sum of the frequency
 * of the time unit edge must be equal to the frequency of the current block!
 */


/**
 * @enum kind_t
 * Defines the type of events, mainly which hardware
 * feature causes the event.
 * @ingroup etime
 */

/**
 * @var kind_t NONE
 * Special null event kind.
 */

/**
 * @var kind_t FETCH
 * Event arising on the fetch stage of the instruction.
 */

/**
 * @var kind_t MEM
 * Event arising on the stage performing access to memory for data.
 */

/**
 * @var kind_t BRANCH;
 * Event arising on the stage performing the branch.
 */

/**
 * @var kind_t CUSTOM;
 * Event arising on a particular stage / functional unit
 * provided by Event::unit().
 */

/**
 * @var kind_t STAGE
 * @deprecated use @ref CUSTOM instead.
 */

/**
 * @var kind_t FU
 * @deprecated use @ref CUSTOM instead.
 */


/**
 * @enum occurence_t
 * Represents the type of occurence of the event.
 */

/**
 * @var occurence_t NEVER
 * The event never happens and the even is mainly here for information.
 */

/**
 * @var occurence_t SOMETIMES
 * The event sometimes happen. The method Event::estimate()
 * provides an estimation of the frequency of the event.
 */

/**
 * @var occurence_t ALWAYS
 * The event always happens and the event object provides
 * mainly the Event::cost() (in cycles) of the event.
 */
io::Output& operator<<(io::Output& out, occurrence_t occ) {
	static cstring labels[] = {
		"never",
		"sometimes",
		"always"
	};
	out << labels[int(occ)];
	return out;
}


/**
 * @enum type_t
 * Define the type of application of the event in the
 * pipeline execution.
 */

/**
 * @var type_t LOCAL
 * The cost is only applied to the stage duration where the event arises.
 */

/**
 * @var type_t AFTER
 * The event cost means that the stage event starts after a particular stage end
 * provided by Event::related() method.
 */

/**
 * @var type_t NOT_BEFORE
 * The event cost means that the stage event starts at the same time as a particular stage
 * provided by Event::related() method.
 */

/**
 * @var type_t EDGE
 * @deprecated	Same as @ref AFTER.
 */

/**
 * @var type_t BLOCK
 * @deprecated	Same as @ref BLOCK.
 */


/**
 * This feature ensures that events of the following analyses has been hooked to the task basic blocks:
 * @li L1 instruction cache by category,
 * @li L1 data cache by category,
 * @li branch prediction by categrory.
 *
 * @par Properties
 * @li @ref EVENT
 *
 * @ingroup etime
 */
p::feature STANDARD_EVENTS_FEATURE("otawa::etime::STANDARD_EVENTS_FEATURE", new Maker<StandardEventBuilder>());


/**
 * This feature ensures that all timing events on pipeline has been recorded.
 *
 * @par Properties
 * @li @ref EVENT
 * @li @ref PREFIX_EVENT
 *
 * @par Default Processor
 * @li @ref StandardEventBuilder
 *
 * @ingroup etime
 */
p::feature EVENTS_FEATURE("otawa::etime::EVENTS_FEATURE", new Maker<StandardEventBuilder>());


/**
 * Allows to hook an event to a basic block
 * or to an edge.
 *
 * @par Feature
 * @li @ref STANDARD_EVENTS_FEATURE
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @li @ref Edge
 *
 * @ingroup etime
 */
p::id<Event *> EVENT("otawa::etime::EVENT", 0);


/**
 * Allows to hook an event to an edge and consider it as part
 * of the prefix of the edge time.
 *
 * @par Feature
 * @li @ref STANDARD_EVENTS_FEATURE
 *
 * @par Hooks
 * @li @ref Edge
 *
 * @ingroup etime
 */
p::id<Event *> PREFIX_EVENT("otawa::etime::PREFIX_EVENT", 0);


/**
 * @class Event
 * An event represents a time variation in the execution of an instruction.
 * Examples of events include instruction/data cache hit/miss, resolution
 * of a branch prediction, hit/miss in the prefetch device of flash memory, etc.
 *
 * Events are used to compute the execution time of code blocks
 * and are usually linked to their matching block.
 *
 * @ingroup etime
 */


/**
 * Build an event.
 * @param inst	Instruction it applies to.
 */
Event::Event(Inst *inst): _inst(inst) {
}


/**
 */
Event::~Event(void) {
}


/**
 * @fn Inst *Event::inst(void) const;
 * Get the instruction this event applies to.
 * @return	Event instruction.
 */


/**
 * @fn kind_t Event::kind(void) const;
 * Get the kind of the event.
 * @return	Event kind.
 */


/**
 * Get the occurrences class of this event.
 * @return	Event occurrence class.
 */
occurrence_t Event::occurrence(void) const {
	return SOMETIMES;
}


/**
 * Get the name of the event (for human user).
 * @return	Event name.
 */
cstring Event::name(void) const {
	return "";
}


/**
 * @fn ot::time Event::cost(void) const;
 * Get the cost in cycles of the occurrence of the event.
 * @return	Cost of the event (in cycles).
 */


/**
 * Ask for support of overestimation for the event when activated (on is true)
 * or deactivated (on is false).
 *
 * May be overridden according to the actual event. As a default, return false.
 * @param on	Test for event activated, or not activated.
 * @return		True if the event provides support for the activation.
 */
bool Event::isEstimating(bool on) {
	return false;
}


/**
 * Weight is a coarse-grain estimation of the number of times an event arises.
 * As a default returns 1 but may be customized to get more precise weight estimation.
 * This result will be used by heuristic approach to assess the impact of this event.
 * @return	Weight of the event.
 */
int Event::weight(void) const {
	return 1;
}


/**
 * Add an estimation of the event count at the right of the given constraint.
 * If the event is considered on, the left value must be an overestimation.
 * If the event is considered off, the left value must be an underestimation.
 *
 * May be overridden to provide specific behavior for the actual event.
 * As a default, do nothing.
 *
 * @param cons	Constraint to add overestimation to.
 * @param on	Add overestimation when the event is triggered (true) or not triggered (false).
 */
void Event::estimate(ilp::Constraint *cons, bool on) {
}


/**
 * For an AFTER or NOT_BEFORE event, gives the pair (instruction, stage)
 * the event is relative to.
 * @return	The (instruction, stage) the event is relative to.
 */
Event::rel_t Event::related(void) const {
	return pair(null<Inst>(), null<const hard::PipelineUnit>());
}


/**
 * For CUSTOM kind, gives the unit the event applies to.
 * @param	Unit the event applies to.
 */
const hard::PipelineUnit *Event::unit(void) const {
	return null<hard::PipelineUnit>();
}


/**
 * Get details about the event to display to the human user.
 * @return	Human readable details about the event.
 */
string Event::detail(void) const {
	return "";
}


/**
 */
io::Output& operator<<(io::Output& out, Event *event) {
	out << event->inst()->address() << " " << event->name()
		<< " (" << event->detail() << ")";
	return out;
}

/**
 */
io::Output& operator<<(io::Output& out, place_t place) {
	static cstring labs[] = {
		"none",
		"prefix",
		"block"
	};
	out << labs[place];
	return out;
}


/**
 * @class Unit
 * An Time Unit (TU) represents the unit of time evaluation of etime. It corresponds
 * mainly to a path in the CFG for which the time has to be computed. This path
 * is split in two parts:
 *	* first part (from the beginning to the pivot edge) is the context or the
 *	  prefix(to provide a state as precise as possible to the time calculation),
 *	* second part (from the pivot edge to the end) represents measured/computed blocks,
 *	  that is, the blocks for which a time is calculated and inserted in
 *	  the WCET calculation.
 *
 * There are different ways to build TUs. The default implementation
 * (@ref TimeUnitBuilder) builds very small TU made of one edge between the previous
 * BB (context / prefix) and the computed BB. When the previous block is not a BB
 * (entry or call block), several edges are looked back (in callers or in called CFG)
 * until finding a BB.
 *
 * @ingroup etime
 */

/**
 * @fn Edge *Unit::pivot(void) const;
 * Get the edge pivot between the context part of the path and the measured block part.
 * Source vertex of this edge is inthe context part and sink vertex is in the measured part.
 * @return	Pivot edge.
 */

/**
 * @fn const List<Edge *> Unit::path(void) const;
 * Get the list of edges making the unit path in order (from first edge of the path,
 * to the last edge).
 * @return	Edges mading the path of the unit.
 */

/**
 * @fn const List<Event *> Unit::events(void) const;
 * Get the list of events applying to the time unit.
 * @return	Time unit edges (not ordered in any way).
 */


/**
 * Provide the basic implementation of @ref TIME_UNIT_FEATURE based
 * on the description of the @ref etime module.
 *
 * Time units are built as below:
 * 	* block part is composed of one vertex, *v*
 * 	* for each basic block predecessor, *v*, one-edge time unit is created: [*w* -> *v*]
 * 	* if a predecessor *w* is the entry of the task, one edge time unit is also created: [*w* -> *v*]
 * 	* if a predecessor *w* is a synthetic block but callee CFG is unknown,  one-edge time unit is created: [*w* -> *v*]
 * 	* if a predecessor *w* is a synthetic and but callee CFG is known, callee blocks are looked back until finding
 * 	  a basic block (or possibly entering again a called CFG): this may create several time units made of the traversed
 * 	  edges,
 * 	* if a predecessor *w* is the entry of CFG (not of the task), the caller CFG blocks are looked back until
 * 	  finding a basic block (possibly entering again a called CFG): also, this may create several time units made of
 * 	  the traversed edges.
 *
 * One can observe that the time units will be made of two BB: the computed block and, if available, one context BB.s
 */
class TimeUnitBuilder: public BBProcessor {
public:
	static p::declare reg;
	TimeUnitBuilder(p::declare& r = reg): BBProcessor(reg) { }

protected:

	static Unit *make(Edge *e) {
		Unit *tu = new Unit(e);
		tu->add(e);
		return tu;
	}

	static Unit *add(Unit *tu, Edge *e) {
		tu->add(e);
		return tu;
	}

	static Unit *copy(Unit *ctu) {
		Unit *tu = new Unit(ctu->edge());
		Vector<Edge *> es;
		for(auto e: ctu->contribs())
			es.push(e);
		while(es)
			tu->add(es.pop());
		return tu;
	}

	void complete(Unit *tu) {
		if(logFor(LOG_INST))
			logTU(tu);
		TIME_UNIT(tu->edge()->sink()).add(tu);
	}

	void logTU(Unit *tu) {
		log << "\t\t\tadded TU = [";
		bool fst = true;
		for(auto i: tu->path()) {
			if(fst)
				fst = false;
			else
				log << ", ";
			log << i;
		}
		log << "]\n";
	}

	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
		if(!b->isBasic())
			return;
		BasicBlock *v = b->toBasic();
		for(Block::EdgeIter e = v->ins(); e; e++) {
			Block *w = e->source();
			lookBack(make(e), w);
		}
	}

private:

	void lookBack(Unit *tu, Block *w) {
		ASSERT(!w->isExit());

		if(w->isBasic())			// ending BB
			complete(tu);

		else if(w->isEntry()) {		// w is entry
			if(!w->cfg()->callers())	// task entry: stop
				complete(tu);
			else {						// CFG entry: look back callers
				for(auto c: w->cfg()->callers())
					for(auto e: c->inEdges())
						lookBack(add(copy(tu), e), e->source());
				delete tu;
			}
		}

		else {						// w is synthetic
			ASSERT(w->isSynth());
			CFG *g = w->toSynth()->callee();

			if(g == nullptr)			// unknown callee: stop here
				complete(tu);

			else {						// know callee: look back before exit
				for(auto e: g->exit()->inEdges())
					lookBack(add(copy(tu), e), e->source());
				delete tu;
			}
		}
	}

};

/**
 */
p::declare TimeUnitBuilder::reg = p::init("otawa::etime::TimeUnitBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.make<TimeUnitBuilder>()
	.provide(etime::UNIT_FEATURE);


/**
 * This feature ensures that the time units has been built for each basic block.
 *
 * @par Properties
 * @li @ref TIME_UNIT
 *
 * @ingroup etime
 */
p::feature UNIT_FEATURE("otawa::etime::UNIT_FEATURE", p::make<TimeUnitBuilder>());


/**
 * Provides the time unit hooked to a basic block. Depending on the number of prefixes
 * to compute the time of a block, the basic block may have zero to several of
 * these annotations tied.
 *
 * @par Hooks
 * @li @ref BasicBlock
 *
 * @par Features
 * @li @ref TIME_UNIT_FEATURE
 *
 * @ingroup etime
 */
p::id<Unit *> TIME_UNIT("otawa::etime::TIME_UNIT", 0);


class Plugin: public ProcessorPlugin {
public:
	Plugin(void): ProcessorPlugin("otawa::etime", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
};

} }	// otawa::etime

otawa::etime::Plugin otawa_etime;
ELM_PLUGIN(otawa_etime, OTAWA_PROC_HOOK);
