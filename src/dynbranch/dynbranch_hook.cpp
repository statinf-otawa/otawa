/*
 *	dynbranch plugin hook
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

namespace otawa { namespace dynbranch {

/**
 * @defgroup dynbranch dynbranch Plugin
 *
 * This plugins implements a relatively simple and inefficient
 * algorithm to resolve indirect branch. It is based on the assumption
 * that a value, used as a branch address, is not processed with computation;
 * always the result of constant processing. It does not work with complex cases
 * where, for example, a branch address is assigned in a loop.
 *
 * @par Plugin Information
 * @li name: otawa/dynbranch
 * @li header: none
 *
 */

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::dynbranch", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }		// otawa::cg

otawa::dynbranch::Plugin OTAWA_PROC_HOOK;
otawa::dynbranch::Plugin& otawa_dynbranch = OTAWA_PROC_HOOK;
