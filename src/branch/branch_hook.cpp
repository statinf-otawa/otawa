/*
 *	branch plugin hook
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

namespace otawa { namespace branch {

/**
 * @defgroup branch branch Plugin
 *
 * This plugins implements the analysis for branch predictor proposed in paper below:
 *
 * A. Colin, I. Puaut.
 * Worst case execution time analysis for a processor with branch prediction
 * (BTB with 2-bit counter and saturation).
 * Real-Time Systems, Special issue on worst-case execution time analysis, 2000.
 *
 * This analysis assigns a category to each branch that can be used to evaluate
 * a bound on the number of miss-predictions.
 *
 * @par Plugin Information
 * @li name: otawa/branch
 * @li header: <otawa/branch/features.h>
 *
 */

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::branch", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }		// otawa::cg

otawa::branch::Plugin OTAWA_PROC_HOOK;
otawa::branch::Plugin& otawa_branch = OTAWA_PROC_HOOK;
