/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/BBProcessor.h -- BBProcessor class implementation.
 */

#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg.h>

namespace otawa {

/**
 * @class BBProcessor
 * This processor is dedicated to the basic block process thru proccessBB()
 * method. Yet, it may also be applied to CFG and framework. In this case,
 * it is applied at each basic block from these higher structures.
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
BBProcessor::BBProcessor(elm::String name, elm::Version version)
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
