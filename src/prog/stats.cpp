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
 * Return a symbolic identifier to design the statistic set.
 * Return the name as a default implementation.
 * @return	Symbolic identifier.
 */
cstring StatCollector::id(void) const {
	return name();
}

/**
 * Get the group of the statistics collector.
 * Default implementation returns empty set.
 * @return	Group owning this stats.
 */
cstring StatCollector::group(void) const {
	return "";
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
 * @fn cstring StatCollector::unit(void) const;
 * Name of statistics unit (for human user, in english).
 */


/**
 * @fn bool StatCollector::isEnum(void) const;
 * Test if the statistics concerns enumerated value.
 * If this method returns true, method valueName() can be called to collect
 * names of enumerated values. The number of enumerated values is given by
 * the total method.
 * @return	True if the statistics are enumerated values.
 */


/**
 * @fn const cstring StatCollector::valueName(int value);
 * Get the name of an enumerated value (for human user, in english).
 * May only be called if isEnum() returns true.
 * @param value		Enumerated value.
 * @return			Name of the enumerated value.
 */


/**
 * @fn int StatCollector::total(void) const;
 * Get the total value for the current statistics (to compute ratio for example).
 * @return		Total value.
 */


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
 * @fn int StatCollector::mergeContext(int v1, int v2);
 * This function allows to merge statistics of a same block in different contexts.
 */


/**
 * @fn int StatCollector::mergeAgreg(int v1, int v2);
 * This function allows to merge statistics when different blocks are aggregated.
 */


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


genstruct::Vector<StatCollector *>& StatInfo::get(WorkSpace *ws) {
	static genstruct::Vector<StatCollector *> empty;
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
StatInfo::Iter::Iter(WorkSpace *ws): genstruct::Vector<StatCollector *>::Iterator(get(ws)) {
}

}	// otawa
