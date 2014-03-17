/*
 *	$Id$
 *	BBProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @class BBProcessor
 * This processor is dedicated to the basic block process thru proccessBB()
 * method. Yet, it may also be applied to CFG and framework. In this case,
 * it is applied at each basic block from these higher structures.
 * @ingroup proc
 */


/**
 * Build a new basic block processor.
 */
BBProcessor::BBProcessor(void) {
}


/**
 * Buid a new named basic block processor.
 * @param name		Processor name.
 * @param version	Processor version.
 */
BBProcessor::BBProcessor(cstring name, elm::Version version)
: CFGProcessor(name, version) {
}


/**
 * @fn void BBProcessor::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
 * Perform the work of the given basic block.
 * @param fw	Container framework.
 * @param cfg	Parent CFG.
 * @param bb	Basic block to process.
 */


/**
 * See @ref CFGProcessor::processCFG()
 */
void BBProcessor::processCFG(WorkSpace *fw, CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {		
		if(logFor(LOG_BB))
			log << "\t\tprocess BB " << bb->number()
				<< " (" << ot::address(bb->address()) << ")\n";
		processBB(fw, cfg, bb);
	}
}


/**
 */
void BBProcessor::cleanupCFG(WorkSpace *ws, CFG *cfg) {
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
void BBProcessor::cleanupBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
}


/**
 * @class BBCleaner
 * Efficient implementation of a cleaner for properties found on a basic block.
 * To instantiate it, juste overload the process() methods
 */

/**
 */
void BBCleaner::clean(void) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	for(CFGCollection::Iterator cfg(coll); cfg; cfg++)
		for(CFG::BBIterator bb(cfg); bb; bb++)
			clean(ws, cfg, bb);
}

/**
 * @fn void BBProcessor::trackBB(const AbstractFeature& feature, const Identifier<T *>& id);
 * Track and delete the content of the given identifier on each basic block.
 * @param feature	Feature this identifier is linked to.
 * @param id		Identifier of the properties to remove.
 */


/**
 * @fn void BBProcessor::trackBB(const AbstractFeature& feature, const Identifier<T>& id)
 * Track and remove properties put on basic blocks.
 * @param feature	Feature this identifier is linked to.
 * @param id		Identifier of the properties to remove.
 */


/**
 * @class BBDeletor
 * Basic block cleaner removing an identifier and deleting its content.
 * @param T		Type of data pointed by the identifier.
 */

/**
 * @class BBCleaner
 * Basic block cleaner simply removing an identifier from basic blocks.
 * @param T		Type of property value.
 */



} // otawa
