/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/proc_LBlockProcessor.cpp -- LBlockProcessor class implementation.
 */

#include <otawa/proc/LBlockProcessor.h>
#include <otawa/otawa.h>
#include <otawa/util/LBlockBuilder.h>

namespace otawa {

/**
 * @class LBlockProcessor
 * This is a specialization of the processor class dedicated to LBlock
 *
 * @note This processor automatically call @ref LBlockBuilder
 */


/**
 * Build a new LBlock processor.
 */
LBlockProcessor::LBlockProcessor(void) {
	require(COLLECTED_LBLOCKS_FEATURE);
}


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 */
LBlockProcessor::LBlockProcessor(elm::String name, elm::Version version)
: Processor(name, version) {
	require(COLLECTED_LBLOCKS_FEATURE);
}


/**
 */
void LBlockProcessor::processWorkSpace(WorkSpace *fw) {

	// Visit LBlocks
}

/**
 * Initialize the processor.
 * @param props	Configuration properties.
 */
void LBlockProcessor::init(const PropList& props) {
}


/**
 * Configure the current processor.
 * @param props	Configuration properties.
 */
void LBlockProcessor::configure(const PropList& props) {
	Processor::configure(props);
	init(props);
}

} // otawa
