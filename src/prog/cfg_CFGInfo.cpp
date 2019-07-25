/*
 *	CFGInfo class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2004-16, IRIT UPS.
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

#include <elm/assert.h>

#include <otawa/cfg.h>
#include <otawa/cfg/CFGBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/instruction.h>
#include <otawa/manager.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prop/DeletableProperty.h>

namespace otawa {

/**
 * @class CFGInfo
 * This allows storing all CFG available in a workspace.
 *
 * @ingroup cfg
 */


/**
 * This property allows to get the CFG information from the current CFG is member of.
 *
 * @par Hooks
 * @li @ref CFG
 *
 * @see CFG_INFO
 * @ingroup cfg
 */
Identifier<const CFGInfo *>& CFGInfo::ID = CFG_INFO;


/**
 * Build a new CFGInfo.
 * @param ws	Workspace that the CFG information applies to.
 */
CFGInfo::CFGInfo(WorkSpace *ws): _ws(ws) {
}


/**
 * Delete the CFG information contained in this program.
 */
CFGInfo::~CFGInfo(void) {
	clear();
}


/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {
	for(Iter g(this); g(); g++)
		delete *g;
	_cfgs.clear();
}


/**
 * @fn CFG *CFGInfo::findCFG(Inst *inst) const;
 * Find the CFG starting by the basic block containing this instruction.
 * @param inst	Instruction to find the CFG starting with.
 * @return 		Matching CFG or null.
 */


/**
 * Get a CFG from the address of its first instruction.
 * @param addr	Address of the first instruction.
 * @return		Found CFG or null.
 */
CFG *CFGInfo::findCFG(Address addr) const {
	return _cfgs.get(addr, static_cast<CFG *>(0));
}


/**
 * Find the CFG starting at the given label.
 * @param label		Label of the first instruction of the CFG.
 * @return			Matching CFG or null.
 */
CFG *CFGInfo::findCFG(String label) {
	Inst *inst = _ws->process()->findInstAt(label);
	if(!inst)
		return 0;
	else
		return findCFG(inst);
}


/**
 * Add a CFG to the CFG information structure.
 * @param cfg	Added CFG.
 */
void CFGInfo::add(CFG *cfg) {
	ASSERTP(cfg, "null CFG given");
	_cfgs.put(cfg->address(), cfg);
}


/**
 * Get the collection of CFG found in the program.
 *
 * @par Feature
 * @li @ref CFG_INFO_FEATURE
 *
 * @par Hooks
 * @ref @ref WorkSpace
 *
 * @ingroup cfg
 */
Identifier<const CFGInfo *> CFG_INFO("otawa::CFG_INFO", 0);

/**
 * Feature asserting that the CFG has been scanned in the program. The result
 * is put the @ref CFGInfo::ID.
 *
 * @Properties
 * @li @ref CFG_INFO
 *
 * @ingroup cfg
 */
p::feature CFG_INFO_FEATURE("otawa::CFG_INFO_FEATURE", new Maker<CFGBuilder>());


} // otawa
