/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_CFGProcessor.cpp -- CFGProcessor class implementation.
 */

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>

namespace otawa {

/**
 * @class CFGProcessor
 * This is a specialization of the processor class dedicated to CFG
 * processing. The @ref Processor::processFrameWork() method just take each
 * CFG of the framework and apply the processor on.
 */

/**
 * @fn void CFGProcessor::processCFG(CFG *cfg);
 * Process the given CFG.
 * @param CFG	CFG to process.
 */


/**
 * Override @ref Processor::processFrameWork().
 */
void CFGProcessor::processFrameWork(FrameWork *fw) {
	
	// Get CFG information
	CFGInfo *info = fw->getCFGInfo();
	
	// For each CFG
	for(Iterator<CFG *> cfg(info->cfgs()); cfg; cfg++)
		processCFG(cfg);
}

} // otawa
