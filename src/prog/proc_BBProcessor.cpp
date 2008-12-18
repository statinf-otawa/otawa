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
 * @param CFG	Parent CFG.
 * @param bb	Basic block to process.
 */


/**
 * See @ref CFGProcessor::processCFG()
 */
void BBProcessor::processCFG(WorkSpace *fw, CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {		
		if(isVerbose())
			log << "\t\tprocess BB " << bb->number()
				<< " (" << fmt::address(bb->address()) << ")\n"; 
		processBB(fw, cfg, bb);
	}
}

} // otawa
