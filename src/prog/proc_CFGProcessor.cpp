/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/proc_CFGProcessor.cpp -- CFGProcessor class implementation.
 */

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/otawa.h>
#include <otawa/cfg/CFGCollector.h>

namespace otawa {

// CFG Queue Marker
static GenericIdentifier<CFG *> QUEUE_NODE("otawa.ipet.queue_node");


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
CFGProcessor::CFGProcessor(const PropList& props)
: Processor(props), last(0) {
	init(props);
}


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 */
CFGProcessor::CFGProcessor(elm::String name, elm::Version version,
const PropList& props): Processor(name, version, props), last(0) {
	init(props);
}


/**
 */
void CFGProcessor::processFrameWork(FrameWork *fw) {

	// Get the CFG collection
	CFGCollection *cfgs = INVOLVED_CFGS(fw);
	if(!cfgs) {
		CFGCollector collector;
		collector.process(fw);
		cfgs = INVOLVED_CFGS(fw);
	}
	assert(cfgs);

	// Visit CFG
	int count = 0;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++) {
		if(isVerbose())
			out << "\tprocess CFG " << cfg->label() << io::endl;
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
GenericIdentifier<CFG *> ENTRY_CFG("otawa.proc.entry_cfg", 0);


/**
 * This property is used to store statistics about the count of processed
 * CFG.
 */
GenericIdentifier<int> PROCESSED_CFG("otawa.proc.processed_cfg", 0);


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
	init(props);
}

/**
 * Activate the recucursive feature of BBProcessors : each time a basic block
 * contains a function call, the CFG of this function is recorded to be
 * processed later after the current CFG. Note that each function is only
 * processed once !
 */
GenericIdentifier<bool> RECURSIVE("otawa.recursive", false);

} // otawa
