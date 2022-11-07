/*
 *	Event class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2021, IRIT UPS.
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

#include <otawa/events/Event.h>
#include <otawa/events/StandardEventBuilder.h>
#include <otawa/prog/Inst.h>

namespace otawa {

/**
 * @defgroup events		Event Management
 * In OTAWA, an event is an internal effecy (usually from the hardware) causing
 * variable execution time. Common events include cache accesses, branch prediction,
 * variable duration instructions, etc).
 * 
 * Events are defined by:
 * * their latency (when they arise)
 * * an estimation of their occurrences (in IPET system)
 * * the pipeline unit they apply to,
 * * their type (how they relates to pipeline stages).
 * 
 * Events provides a very open and flexible system to relates the global hardware
 * effects produced by global analyses with the microarchitecture time calculation
 * algorithms.
 */

/**
 * @enum Event::kind_t
 * Defines the type of events, mainly which hardware
 * feature causes the event.
 * @ingroup events
 */

/**
 * @var Event::kind_t Event::NONE
 * Special null event kind.
 */

/**
 * @var Event::kind_t Event::FETCH
 * Event arising on the fetch stage of the instruction.
 */

/**
 * @var Event::kind_t Event::MEM
 * Event arising on the stage performing access to memory for data.
 */

/**
 * @var Event::kind_t Event::BRANCH;
 * Event arising on the stage performing the branch.
 */

/**
 * @var Event::kind_t Event::CUSTOM;
 * Event arising on a particular stage / functional unit
 * provided by Event::unit().
 */


/**
 * @enum Event::occurence_t
 * Represents the type of occurence of the event.
 */

/**
 * @var Event::occurence_t Event::NEVER
 * The event never happens and the even is mainly here for information.
 * @ingroup events
 */

/**
 * @var Event::occurence_t Event::SOMETIMES
 * The event sometimes happen. The method Event::estimate()
 * provides an estimation of the frequency of the event.
 * @ingroup events
 */

/**
 * @var Event::occurence_t Event::ALWAYS
 * The event always happens and the event object provides
 * mainly the Event::cost() (in cycles) of the event.
 * @ingroup events
 */

///
io::Output& operator<<(io::Output& out, Event::occurrence_t occ) {
	static cstring labels[] = {
		"never",
		"sometimes",
		"always"
	};
	if(occ == Event::NO_OCCURRENCE)
		out << "<none>";
	else
		out << labels[int(occ)];
	return out;
}

/**
 * Combines two event occcurrences according to the partial order (getting the
 * maximu of x and y).
 * * for x in occurrence_t, NO_OCCURRENCE <= x
 * * for x in occurence_t, x <= SOMETIMES.
 * 
 * @param x		First occurrence to combine.
 * @param y		Second occurrence to combine.
 * @return		Maximum of x and y.
 * @ingroup events
 */
Event::occurrence_t operator|(Event::occurrence_t x, Event::occurrence_t y) {
	if(x == y)
		return x;
	switch(x) {
	case Event::NO_OCCURRENCE:
		return y;
	case Event::SOMETIMES:
		return Event::SOMETIMES;
	case Event::ALWAYS:
	case Event::NEVER:
		switch(y) {
		case Event::NO_OCCURRENCE:
			return x;
		default:
			return Event::SOMETIMES;
		}
	default:
		ASSERT(0);
		return Event::NO_OCCURRENCE;
	}
}



/**
 * @enum Event::type_t
 * Define the type of application of the event in the
 * pipeline execution.
 */

/**
 * @var Event::type_t Event::LOCAL
 * The cost is only applied to the stage duration where the event arises.
 */

/**
 * @var Event::type_t Event::AFTER
 * The event cost means that the stage event starts after a particular stage end
 * provided by Event::related() method.
 */

/**
 * @var Event::type_t Event::NOT_BEFORE
 * The event cost means that the stage event starts at the same time as a particular stage
 * provided by Event::related() method.
 */


/**
 * @class Event
 * An event represents a time variation in the execution of an instruction.
 * Examples of events include instruction/data cache hit/miss, resolution
 * of a branch prediction, hit/miss in the prefetch device of flash memory, etc.
 *
 * Events are used to compute the execution time of code blocks
 * and are usually linked to their matching block.
 *
 * @ingroup event
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
Event::occurrence_t Event::occurrence(void) const {
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
bool Event::isEstimating(bool on) const {
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
void Event::estimate(ilp::Constraint *cons, bool on) const {
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
 * @ingroup events
 */
p::id<Event *> EVENT("otawa::EVENT", 0);


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
 * @ingroup events
 */
p::id<Event *> PREFIX_EVENT("otawa::PREFIX_EVENT", 0);


/**
 * This feature ensure that basic events has been built from classic global anayzes.
 * 
 * **Default implementation**: StandardEventBuilder
 * 
 * **Hooked properties**
 * * EVENT
 * * PREFIX_EVENT
 * 
 * @ingroup events
 */
//p::feature STANDARD_EVENT_FEATURE("otawa::STANDARD_EVENT_FEATURE", p::make<StandardEventBuilder>());

} // otawa
