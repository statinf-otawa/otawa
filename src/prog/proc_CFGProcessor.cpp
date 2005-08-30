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
 * Build a new CFG processor.
 * @param props		Configuration properties.
 */
CFGProcessor::CFGProcessor(const PropList& props): Processor(props) {
}


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 */
CFGProcessor::CFGProcessor(elm::String name, elm::Version version,
const PropList& props): Processor(name, version, props) {
}


/**
 * @fn void CFGProcessor::processCFG(FrameWork *fw, CFG *cfg);
 * Process the given CFG.
 * @param fw	Container framework.
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
		processCFG(fw, cfg);
}

} // otawa
