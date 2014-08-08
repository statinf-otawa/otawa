/*
 *	CCGPlugin hook
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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

namespace otawa { namespace ccg {

/**
 * @defgroup ccg CCG Plugin
 *
 * CCG (Cache Conflict Graph) provides a method to compute the number of misses
 * of a direct-mapped instruction cache based on a graph representation
 * of the cache states translated as a set of constraints. An extension exists
 * for A-way LRU caches but is not implemented in OTAWA.
 *
 * Y.-T. S. Li, S. Malik, A. Wolfe.
 * Efficient microarchitecture modelling and path analysis for real-time software.
 * Proceedings of the 16th IEEE Real-Time Systems Symposium, 12/1995.
 *
 * Y.-T. S. Li, S. Malik, A. Wolfe.
 * Cache Modeling for Real-Time Software: Beyond Direct Mapped Instruction Caches
 * Proceedings of ACM SIGPLAN Workshop on Language, Compiler and Tool Support for Real-time Systems, 6/1997.
 *
 * @par Plugin Information
 * @li name: otawa/ccg
 * @li header: <otawa/ccg/features.h>
 *
 * @par Analyses
 * @li @ref ccg::Builder -- build the CCG for each set of the current cache,
 * @li @ref ccg::ConstraintBuilder -- build the constraint representing the CCG in the ILP.
 *
 */

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::ccg", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }		// otawa::cg

otawa::ccg::Plugin OTAWA_PROC_HOOK;
otawa::ccg::Plugin& otawa_ccg = OTAWA_PROC_HOOK;
