/*
 *	$Id$
 *	CFGProcessor class implementation
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

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/otawa.h>
#include <otawa/cfg/CFGCollector.h>

namespace otawa {

/**
 * @class CFGProcessor
 * This is a specialization of the processor class dedicated to CFG
 * processing. The @ref Processor::processFrameWork() method just take each
 * CFG of the framework and apply the processor on.
 * 
 * It accepts in configuration the following properties:
 * @li @ref ENTRY_CFG: the entry CFG of the task to work with,
 * @li @ref RECURSIVE: to go down recursively in the task CFG.
 * 
 * If statistics are required, it provides:
 * @li @ref PROCESSED_CFG: records the count of processed CFG.
 *
 * Finally, it puts on the framework the following properties:
 * @li @ref INVOLVED_CFGS: collection of CFG used by the current task.
 *
 * @note This processor automatically call @ref CFGCollector.
 */


// registration
void CFGProcessor::init(void) {
	_name("otawa::CFGProcessor");
	_version(1, 0, 0);
	_require(COLLECTED_CFG_FEATURE);	
}


/**
 * Build a new CFG processor.
 */
CFGProcessor::CFGProcessor(void) {
}


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 */
CFGProcessor::CFGProcessor(cstring name, elm::Version version)
: super(name, version) {
}


/**
 */
void CFGProcessor::processWorkSpace(WorkSpace *fw) {

	// Get the CFG collection
	CFGCollection *cfgs = INVOLVED_CFGS(fw);
	assert(cfgs);

	// Visit CFG
	int count = 0;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++) {
		if(isVerbose())
			log << "\tprocess CFG " << cfg->label() << io::endl;
		processCFG(fw, cfg);
		count++;
	}
	
	// Record stats
	if(recordsStats())
		PROCESSED_CFG(stats) = count;	
}


/**
 * This property may be used to pass the entry CFG to a CFG processor or
 * is used by the CFG processor to record in the framework the entry CFG
 * of the currently processed task.
 */
Identifier<CFG *> ENTRY_CFG("otawa::entry_cfg", 0);


/**
 * This property is used to store statistics about the count of processed
 * CFG.
 */
Identifier<int> PROCESSED_CFG("otawa::processed_cfg", 0);


/**
 * Initialize the processor.
 * @param props	Configuration properties.
 */
void CFGProcessor::init(const PropList& props) {
}


/**
 * Configure the current processor.
 * @param props	Configuration properties.
 */
void CFGProcessor::configure(const PropList& props) {
	Processor::configure(props);
	init(props);
}

/**
 * Activate the recucursive feature of BBProcessors : each time a basic block
 * contains a function call, the CFG of this function is recorded to be
 * processed later after the current CFG. Note that each function is only
 * processed once !
 */
Identifier<bool> RECURSIVE("otawa::recursive", true);

} // otawa
