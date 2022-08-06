/*
 *	StatInfo class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011-17, IRIT UPS.
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

#include <otawa/stats/StatInfo.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @page proc_stats Statistics Collection
 * @ingroup proc
 * 
 * Code processor aim to perform analyses on the programs. To reflect the
 * work of these analyses or to sum-up the results, numeric can be produced
 * and displayed to the human user. The goal of this sub-module is to provide
 * a common way to expose statistics of a code processor.
 * 
 * @par Usage
 * 
 * Basically, a processor can provide zero or several statistics to expose to
 * the human user. For each statistics, an implementation of the interface
 * @ref StatCollector has to be provided.
 * 
 * This interface provides all details to generate a display for the user.
 * It contains:
 * * an internal identifier (for example "wcet-time"),
 * * a human-readable label ("Execution Time"),
 * * a human-readable unit ("cycle")
 * * a set of keywords ("WCET, "time")
 * * a description ("Statistic giving the execution time of code parts.")
 * * a total, typically for percentage calculation.
 * 
 * The most important function is the @ref StatCollector::collect() function that
 * is called when the statistics will be exploited. It takes as parameter an
 * interface, @ref StatCollector::Collector that contains one function, 
 * @ref StatCollector::Collector::collect() that has to be called for each
 * statistics item.
 * 
 * So to provide a statistics, one has to provide all or partially the details
 * described above and the @ref StatCollector::collect() function. This function
 * has to traverse the program representation and for each statistics item has
 * to call @ref StatCollector::Collector::collect(). In turn, the application
 * that aims to collect statistics has to implement @ref StatCollector::Collector
 * and to call @ref StatCollector::collect() to get the statistics.
 * 
 * To provide statistics, one has only to implement the function
 * @ref Processor::collectStat() and to call the function @ref Processor::record()
 * for each provided statistics with the instance of the class implementing
 * the statistics collection:
 * @code
 *	class TotalCountStat: public StatCollector { ... };
 *	class EdgeTotalTimeStat: public StatCollector { ... };
 *	class TotalTimeStat: public StatCollector { ... };
 * 
 *	void MyProcessor::collectStats(WorkSpace *ws) {
 *		record(new TotalCountStat);
 *		record(new EdgeTotalTimeStat);
 *		record(new TotalTimeStat);
 * 	}
 * @endcode
 * 
 * This function is automatically called when statistics are required. Notice
 * that the processor is in charge of freeing the instance passed to
 * @ref Processor::record().
 * 
 * Notice that, basically, the statistics are independent of the program
 * representation but a statistics item applies to one particular part of
 * thes program. To stay independent of the representation, the program
 * piece is identifierd by:
 * * its base address,
 * * it size in bytes,
 * * its context (@ref otawa::Context).
 * 
 * @par Merging
 * 
 * The data collected by @ref StatCollector can be exposed to the human user
 * according to different modes that may mean that some statistics item
 * have to be aggregate. For example, with a source display, a source line
 * may correspond to different statistics items.
 * 
 * There exist different ways to perform this aggregation:
 * * @ref StatCollector::SUM_OP - addition,
 * * @ref StatCollector::MAX_OP - maximium,
 * * @ref StatCollector::MIN_OP - minimum.
 * 
 * The @ref StatCollector interface provides several way to specify the
 * aggregation operations for different situation:
 * * @ref StatCollector::concatOperation - when two code part are concatenated,
 * * @ref StatCollector::contextOperation - when the same code part is
 * 		aggreagated with different contexts.
 * * @ref StatCollector::lineOperation - when the statistics are aggregated
 * 		according to source lines.
 * 
 * Clearly, these aggregations, most often, cannot reflect precisely the
 * statistics because of the merge of program structure depending on the
 * program representation but merely the statistics display is provided
 * to human user and may accept some imprecision.
 * 
 * In the example of the execution time, the aggregation operations could be:
 * * @ref StatCollector::concatOperation - addition,
 * * @ref StatCollector::contextOperation - maximum,
 * * @ref StatCollector::lineOperation - addition.
 * 
 * Concerning the execution count of a code part on the WCET path,
 * * @ref StatCollector::concatOperation - maximum,
 * * @ref StatCollector::contextOperation - maximum,
 * * @ref StatCollector::lineOperation - maximum.
 * 
 * Sometimes, depending on the context, some statistics needs diffderent
 * aggregation operations. In this case, it is simpler to provide several
 * @ref StatCollector instances.
 * 
 * @par Statistics and BB
 * 
 * CFG and BB are first-class citizens in the program representation in OTAWA.
 * Moreover they allows the duplication of BBs according different context
 * (function call chains) or according any CFG transformation algorithm. This
 * means the same code part may exhibit different statistics but with the same
 * location as specified by @ref StatCollector. This results in a loss of
 * precision.
 * 
 * To prevent this, version 1.1 of @ref StatCollector provides a bridge to
 * exploit BB-based statistics using the functions:
 * * @ref StatCollector::supports() - that returns true if the BB interface
 * 		is supported.
 * * @ref StatCollector::collectBB() - a BB-aware implementation of the
 * 		function @ref StatCollector::collect().
 * 
 * Things are even simpler if the statistics collector extends the class
 * @ref BBStatCollector. One has just to overide the function
 * @ref BBStatCollector::getStat().
 */

	
/**
 * @class StatCollector
 * A statistics collector allows to access statistics produced by an analysis.
 * Any specialized statistics provider must implements this class and store an instance
 * in the @ref StatInfo workspace instance. This allows generic processing of statistics
 * by user interface applications.
 *
 * The statistics may be structured by group: a group is mainly an identifier that is shared
 * by several statistics. A group may also be structured by separating each component with a "/"
 * (like a file path). Additionally, statistics may have keywords to help the user to select
 * the interesting statistics to display. This type of information is purely symbolic and
 * is provided to automatic processing of statistics.
 *
 * @ingroup proc
 */

/**
 */
StatCollector::~StatCollector(void) {

}

/**
 * Return a symbolic identifier to design the statistic set. This identifier
 * may be structured according to groups or subgroups containing the statistics.
 * As for file path, the identifier components are separated by "/".
 * Return the name as a default implementation.
 * @return	Symbolic identifier.
 */
cstring StatCollector::id() const {
	return name();
}

/**
 * Keywords representing this state.
 * The default implementation does nothing.
 * @param kws	Vector to store keywords in.
 */
void StatCollector::keywords(Vector<cstring>& kws) {
}


/**
 * @fn cstring StatCollector::name(void) const;
 * Name of the statistics (for human user, in english).
 */


/**
 * Name of statistics unit (for human user, in english).
 * @return	Unit name.
 */
cstring StatCollector::unit() const {
	return "";
}


/**
 * Test if the statistics concerns enumerated value.
 * If this method returns true, method valueName() can be called to collect
 * names of enumerated values. The number of enumerated values is given by
 * the total method.
 * @return	True if the statistics are enumerated values.
 */
bool StatCollector::isEnum() const {
	return false;
}


/**
 * Get the name of an enumerated value (for human user, in english).
 * May only be called if isEnum() returns true.
 * @param value		Enumerated value.
 * @return			Name of the enumerated value.
 */
const cstring StatCollector::valueName(int value) {
	return "";
}


/**
 * Get the total value for the current statistics (to compute ratio for example).
 * @return		Total value.
 */
int StatCollector::total() {
	return 0;
}


/**
 * @class StatCollector::Collector
 * This interface class must be implemented by any program that wants to collect
 * statistics for the current statistics information.
 */


/**
 * @fn void StatCollector::Collector::collect(const Address& address, t::uint32 size, int value);
 * Called for each program block for which a statistics exists.
 * @param address	Block address.
 * @param size		Block size (in bytes).
 * @param value		Statistics value for the block.
 */


/**
 * @fn void StatCollector::collect(Collector& collector);
 * Called to collect the statistics.
 * @param collector		Collector of statistics.
 */


/**
 * This function allows to merge statistics of a same block in different contexts.
 * The default implementation performs the maximum of two values.
 */
int StatCollector::mergeContext(int v1, int v2) {
	return max(v1, v2);
}


/**
 * This function allows to merge statistics when different blocks are aggregated.
 * The default implementation compute the sum.
 */
int StatCollector::mergeAgreg(int v1, int v2) {
	return v1 + v2;
}

/**
 * Get the human-readable label to identify the stat.
 * @return	Stat label.
 */
cstring StatCollector::label() const {
	return name();
}

/**
 * Called to get a humand-readable description of the statistics.
 * The result may be simple text (line returns will be ignored) or HTML code.
 * @return	Statistics description.
 */
cstring StatCollector::description() const {
	return "";
}

/**
 * Get the operation used when stats are merged for a source line.
 * @return	Source line merge operation.
 */
StatCollector::operation_t StatCollector::lineOperation() const {
	return SUM_OP;
}

/**
 * Get the operation used when stats are merged because of context merge.
 * @return	Context merge operation.
 */
StatCollector::operation_t StatCollector::contextOperation() const {
	return SUM_OP;
}

/**
 * Get the operation used when stats are merged because of concatenation.
 * @return	Context merge operation.
 */
StatCollector::operation_t StatCollector::concatOperation() const {
	return SUM_OP;
}

/**
 * Implements the aggregation operation.
 * @param x		First value.
 * @param op	Operation.
 * @param y		Second value.
 * @return		Result of aggregation.
 */
int StatCollector::aggregate(int x, operation_t op, int y) {
	switch(op) {
	case NO_OP:		return 0;
	case SUM_OP:	return x + y;
	case MAX_OP:	return max(x, y);
	case MIN_OP:	return min(x, y);
	default:		ASSERTP(false, "bad aggregate op"); return 0;
	}
}


io::Output& operator<<(io::Output& out, StatCollector::operation_t op) {
	static cstring labs[] = {
		"none",
		"sum",
		"max",
		"min"
	};
	out << labs[op];
	return out;
};


/**
 * Called to get a list of defintions to provide to a stat displayer.
 * For exampel, for a static displagin time of block, the definition of the
 * WCET could be provided: key "WCET", value the WCET itself.
 * @param defs	Map to put the definition in.
 */
void StatCollector::definitions(ListMap<cstring, string>& defs) const {
}

/**
 * Test if the current stat collector implements the @ref collect(BBCollector&)
 * function.
 * @return	True if the current collector implements @ref BBStatCollector.
 */
bool StatCollector::supportsBB() const {
	return false;
}


/**
 * Called to collect stats using BB identification. Can only be called when
 * @ref supportsBB() returns true.
 * @param collector		Collector to collect stat.
 */
void StatCollector::collectBB(BBCollector& collector) {
}


/**
 * @class StatInfo
 * Agregator of statistics put on the workspace. Accessible using @ref StatInfo::ID
 * identifier.
 * @ingroup proc
 */


/**
 * Identifier of statistics information.
 *
 * @par Hooks
 * @li WorkSpace
 */
Identifier<StatInfo *> StatInfo::ID("otawa::StatInfo::ID", 0);


/**
 * Add a statistics collector to the workspace.
 * @param ws	Current workspace.
 * @param stats	Statistics collector to add.
 */
void StatInfo::add(WorkSpace *ws, StatCollector& stats) {

	// get a statistics information
	StatInfo *info = ID(ws);
	if(!info) {
		info = new StatInfo();
		ID(ws) = info;
	}

	// add the statistics
	info->stats.add(&stats);
}


/**
 * Remove a statistics.
 * @param ws	Current workspace.
 * @param stats	Statistics to remove.
 */
void StatInfo::remove(WorkSpace *ws, StatCollector& stats) {

	// get a statistics information
	StatInfo *info = ID(ws);
	if(!info) {
		info = new StatInfo();
		ID(ws) = info;
	}

	// remove the statistics
	info->stats.remove(&stats);
}


const Vector<StatCollector *>& StatInfo::get(WorkSpace *ws) {
	static Vector<StatCollector *> empty;
	StatInfo *info = StatInfo::ID(ws);
	if(!info)
		return empty;
	else
		return info->stats;
}


/**
 * Iterate on statistics collector of the workspace.
 * @param ws	Workspace to work on.
 */
StatInfo::Iter::Iter(WorkSpace *ws): Vector<StatCollector *>::Iter(get(ws)) {
}

}	// otawa
