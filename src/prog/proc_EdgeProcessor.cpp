/*
 *	EdgeProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#include <otawa/proc/EdgeProcessor.h>

namespace otawa {

/**
 * @class EdgeProcessor
 * Abstract processor providing simple traversal of edges of the CFG.
 * With this processor, no assumption can be made on the traversal of edges.
 *
 * To use it, create a subclass of this one and override the method @ref processEdge().
 */


/**
 */
EdgeProcessor::EdgeProcessor(void) {
}

/**
 */
EdgeProcessor::EdgeProcessor(cstring name, elm::Version version): CFGProcessor(name, version) {
}

/**
 */
EdgeProcessor::EdgeProcessor(AbstractRegistration& reg): CFGProcessor(reg) {
}

/**
 */
EdgeProcessor::EdgeProcessor(cstring name, const Version& version, AbstractRegistration& reg): CFGProcessor(name, version, reg) {
}

/**
 */
void EdgeProcessor::processCFG(WorkSpace *ws, CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++)
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			if(logFor(Processor::LOG_BLOCK))
				cerr << "\t\t" << *edge << io::endl;
			processEdge(ws, cfg, edge);
		}
}


/**
 */
void EdgeProcessor::cleanupCFG(WorkSpace *ws, CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++)
		cleanupBB(ws, cfg, bb);
}


/**
 * When the function @ref doCleanup() is called,
 * this function is called for each basic block of the task.
 * As a default do nothing.
 * @warning If you override the @ref cleanupCFG() function, do not forget to perform
 * a call to the CFGProcessor original version.
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 * @param bb	Current BB.
 */
void EdgeProcessor::cleanupBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	for(BasicBlock::OutIterator edge(bb); edge; edge++)
		cleanupEdge(ws, cfg, edge);
}


/**
 * When the function @ref doCleanup() is called,
 * this function is called for each edge of the task.
 * As a default do nothing.
 * @warning If you override the @ref cleanupCFG() function, do not forget to perform
 * a call to the CFGProcessor original version.
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 * @param edge	Current edge.
 */
void EdgeProcessor::cleanupEdge(WorkSpace *ws, CFG *cfg, Edge *edge) {
}


/**
 * @fn void EdgeProcessor::processEdge(WorkSpace *ws, CFG *cfg, Edge *edge);
 * This function is called once for each edge and must be overridden by the subclass.
 * @param ws	Current workspace.
 * @param cfg	Current CFG.
 * @param edge	Current edge.
 */

}	// otawa
