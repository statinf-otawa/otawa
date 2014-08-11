/*
 *	cat plugin hook
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

namespace otawa { namespace cat {

/**
 * @defgroup cat Cat Plugin
 *
 * Implementation of LRU instruction cache analysis by categorisation according to the paper:
 *
 * C. A.Healy, R. D. Arnold, F. Mueller, D. B. Whalley, M. G. Harmon
 * Bounding Pipeline and Instruction Cache Performance.
 * IEEE Transactions on Computers, 1/1999.
 *
 * @par Plugin Information
 * @li name: otawa/cat
 * @li header: <otawa/cat/features.h>
 */

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::cat", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }		// otawa::cg

otawa::cat::Plugin OTAWA_PROC_HOOK;
otawa::cat::Plugin& otawa_cat = OTAWA_PROC_HOOK;
