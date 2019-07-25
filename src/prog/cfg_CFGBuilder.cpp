/*
 *	CFGBuilder processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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

#include <otawa/cfg/CFGBuilder.h>
#include <otawa/cfg.h>
#include <otawa/program.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

namespace otawa {

/**
 * @class CFGBuilder
 * This processor is used for building the set of CFG from the binary program
 * representation. Found CFG are linked to the framework thanks a CFGInfo object
 * stored in property @ref CFG_INFO.
 *
 * @par Provided Features
 * @li @ref CFG_INFO_FEATURE
 *
 * @ingroup cfg
 */


p::declare CFGBuilder::reg = p::init("otawa::CFGBuilder", Version(2, 0, 0))
	.base(AbstractCFGBuilder::reg)
	.provide(CFG_INFO_FEATURE)
	.maker<CFGBuilder>();


/**
 * CFG builder constructor.
 */
CFGBuilder::CFGBuilder(p::declare& r): AbstractCFGBuilder(r) {
}


/**
 */
void CFGBuilder::setup(WorkSpace *ws) {
	AbstractCFGBuilder::setup(ws);
	Inst *start = ws->process()->start();
	if(start) {
		maker(start);
		if(logFor(LOG_FUN))
			log << "\tadded function for _start at " << start->address() << io::endl;
	}
	for(Process::FileIter file(ws->process()); file(); file++)
		for(File::SymIter sym(*file); sym(); sym++)
			if(sym->kind() == Symbol::FUNCTION) {
				Inst *inst = ws->findInstAt(sym->address());
				if(!inst)
					warn(_ << "function symbol " << sym->name() << " at " << sym->address() << " does not match code segment!");
				else {
					maker(inst);
					if(logFor(LOG_FUN))
						log << "\tadded function " << sym->name() << " at " << sym->address() << io::endl;
				}
			}
}


/**
 */
void CFGBuilder::cleanup(WorkSpace *ws) {

	// build the CFG information
	/*CFGInfo *info = new CFGInfo(ws);
	for(Iter maker(*this); maker; maker++)
		info->add(maker->build());
	track(CFG_INFO_FEATURE, CFG_INFO(ws) = info);*/

	// cleanup
	AbstractCFGBuilder::cleanup(ws);
}

} // otawa
