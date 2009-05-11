/*
 *	$Id$
 *	Dynamic processor plugin example
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS <casse@irit.fr>
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

#include <elm/io.h>
#include <otawa/cfg.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Registration.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/proc/Feature.h>

using namespace otawa;

namespace dyn {

// DynProcessor class
class DynProcessor: public Registered<DynProcessor, CFGProcessor> {
public:
	DynProcessor(void) { }

	static void init(void) {
		_name("dyn::DynProcessor");
		_version(1, 0, 0);
	}

protected:
	virtual void processCFG (WorkSpace *ws, CFG *cfg) {
		cout << "found CFG " << cfg->label() << io::endl;
	}
};

// DynPlugin class
class DynPlugin: public ProcessorPlugin {
public:
	DynPlugin(void): ProcessorPlugin("dyn", Version(1, 0, 0), OTAWA_PROC_VERSION) { }

	virtual elm::genstruct::Table<AbstractRegistration *>& processors(void) const {
		return elm::genstruct::Table<AbstractRegistration *>::EMPTY;
	}
};

Identifier<int> ID("dyn::ID", 666);

Feature<DynProcessor> FEATURE("dyn::FEATURE");

} // dyn

dyn::DynPlugin OTAWA_PROC_HOOK;
dyn::DynPlugin& dyn_plugin = OTAWA_PROC_HOOK;
