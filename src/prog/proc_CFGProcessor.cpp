/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/proc_CFGProcessor.cpp -- CFGProcessor class implementation.
 */

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGInfo.h>
#include <otawa/otawa.h>

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
 * @fn void CFGProcessor::processCFG(FrameWork *fw, CFG *cfg);
 * Process the given CFG.
 * @param fw	Container framework.
 * @param CFG	CFG to process.
 */


/**
 */
void CFGProcessor::processFrameWork(FrameWork *fw) {

	// Set first queue node
	CFG *first = ENTRY_CFG(fw);
	if(!first) {
		CFGInfo *info = fw->getCFGInfo();
		first = info->findCFG(name);
		if(!first)
			throw ProcessorException(*this, "cannot find task entry point \"%d.\"", &name);
		ENTRY_CFG(fw) = first;
	}
	QUEUE_NODE(first) = 0;
	last = first;
	
	// Traverse the CFGs
	int count = 0;
	CFG *cfg = first;
	while(cfg) {
		if(isVerbose())
			out << "\tprocess CFG " << cfg->label() << io::endl;
		processCFG(fw, cfg);
		count++;
		cfg = QUEUE_NODE(cfg);
	}

	// Cleanup queue nodes
	if(recordsStats())
		PROCESSED_CFG(stats) = count;
	cfg = first;
	while(cfg) {
		CFG *next = QUEUE_NODE(cfg);
		cfg->removeProp(&QUEUE_NODE);
		cfg = next;
	}
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
	name = PROC_ENTRY(props);
}


/**
 * Add a CFG to the current analysis. The added are processed in turn until
 * no more CFG is added.
 * @param bb	Basic block containing the call.
 * @param cfg	Called CFG.
 */
void CFGProcessor::add(BasicBlock *bb, CFG *cfg) {
	assert(last);
	if(!cfg)
		throw ProcessorException(*this,
			"unresolved function call at %08x", bb->address());
	if(!cfg->hasProp(QUEUE_NODE)) {
		QUEUE_NODE(last) = cfg;
		last = cfg;
	}
}


/**
 * Configure the current processor.
 * @param props	Configuration properties.
 */
void CFGProcessor::configure(const PropList& props) {
	init(props);
}

} // otawa
