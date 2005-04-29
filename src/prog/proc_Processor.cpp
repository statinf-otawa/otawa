/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_Processor.cpp -- Processor class implementation.
 */

#include <otawa/proc/Processor.h>

using namespace elm;
using namespace elm::io;

namespace otawa {

/**
 * @class Processor
 * The processor class is implemented by all code processor. At this level,
 * the argument can only be the full framework. Look at sub-classes for more
 * accurate processors.
 */


/**
 * @fn void Processor::processFrameWork(FrameWork *fw);
 * Process the given framework.
 * @param fw	Framework to process.
 */


/**
 * This property identifier is used for setting the output stream used by
 * the processor for writing messages (information, warning, error) to the user.
 * The data must be of type @ref elm::io::OutStream *.
 */
Identifier Processor::ID_Output("proc.output");


/**
 * This method may be called for configuring a processor thanks to information
 * passed in the property list.
 * @param props	Configuration information.
 */
void Processor::configure(PropList& props) {
	OutStream *out_stream = props.get<OutStream *>(ID_Output, 0);
	if(out_stream)
		;
}

} // otawa
