/*
 *	$Id$
 *	CFGInfo processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2004-08, IRIT UPS.
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

#include <otawa/cfg/features.h>
#include <otawa/instruction.h>
#include <otawa/manager.h>
#include <otawa/cfg.h>
#include <elm/assert.h>
#include <otawa/prop/DeletableProperty.h>

// Trace
//#	define TRACE_CFG_INFO
#if defined(NDEBUG) || !defined(TRACE_CFG_INFO)
#	define TRACE(str)
#else
#		define TRACE(str) \
			{ \
				cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n'; \
				cerr.flush(); \
			}
#endif

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
Identifier<CFGInfo *>& CFGInfo::ID = CFG_INFO;


/**
 * Build a new CFGInfo.
 * @param fw	Workspace that the CFG information applies to.
 */
CFGInfo::CFGInfo(WorkSpace *_fw)
: fw(_fw) {
	TRACE(this << ".CFGInfo::CFGInfo(" << _fw << ")");
}


/**
 * Delete the CFG information contained in this program.
 */
CFGInfo::~CFGInfo(void) {
	TRACE(this << ".CFGInfo::~CFGInfo()");
	clear();
}


/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {

	// remove CFG
	for(int i = 0; i < _cfgs.length(); i++)
		delete _cfgs[i];
	_cfgs.clear();

	// remove edges
	for(FragTable<BasicBlock *>::Iterator bb(bbs); bb; bb++)
		while(true) {
			BasicBlock::OutIterator edge(bb);
			if(edge)
				delete *edge;
			else
				break;
		}

	// remove BB
	for(FragTable<BasicBlock *>::Iterator bb(bbs); bb; bb++)
		delete *bb;
	bbs.clear();
}


/**
 * Find the CFG starting by the basic block containing this instruction.
 * If the CFG does not exists, it is created and added.
 * @par
 * The current algorithm does not allow detecting all CFG due to the irregular
 * nature of the assembly code. Specially, function entries also used as
 * loop head or as jump target of a function without continuation.
 * Using this method, the user may inform the system about other existing CFG.
 * @param inst	Instruction to find the CFG starting with.
 * @return 	Matching CFG or null.
 */
CFG *CFGInfo::findCFG(Inst *inst) {
	const BasicBlock *bb = map.get(inst->address());
	if(!bb)
		return 0;
	else
		return findCFG(bb);
}


/**
 * Find the CFG starting at the given basic block.
 * @param bb	Basic block to look at.
 * @return	Found CFG or this BB is not a CFG start.
 */
CFG *CFGInfo::findCFG(const BasicBlock *bb) {
	return ENTRY(bb);
}


/**
 * Find the CFG starting at the given label.
 * @param label		Label of the first instruction of the CFG.
 * @return			Matching CFG or null.
 */
CFG *CFGInfo::findCFG(String label) {
	Inst *inst = fw->process()->findInstAt(label);
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
	ASSERTP(cfg, "null cfg given");
	_cfgs.add(cfg);
}


/**
 * Add the given BB to the CFG information.
 * @param bb	BB to add.
 */
void CFGInfo::add(BasicBlock *bb) {
	bbs.add(bb);
	map.add(bb);
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
Identifier<CFGInfo *> CFG_INFO("otawa::CFG_INFO", 0);

} // otawa
