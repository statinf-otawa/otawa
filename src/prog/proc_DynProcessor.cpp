/*
 *	$Id$
 *	DynProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/proc/DynProcessor.h>

namespace otawa {

/**
 * @class ProcessorNotFound
 * This exception is thrown when a code processor can not be found.
 * @ingroup dyn
 */

/**
 * Build the exception.
 * @param name	Name of not found processor.
 */
ProcessorNotFound::ProcessorNotFound(string name)
:	otawa::Exception(_ << "processor " << name << " can not be found."),
	_name(name)
{
}

/**
 * @fn const string& ProcessorNotFound::name(void) const;
 * Get the name of the not found processor.
 * @return	Not found processor.
 */


/**
 * @class DynProcessor
 * This pseudo-processor allows to call processor based on their name. It is useful
 * when one does not want or does not can't to include the requested processor.
 * For example, it may be useful when the processor is contained in a plugin.
 * In this case, the plugin will be automatically plugged and used.
 */


/**
 * Find the processor whose name is given.
 * @param name					Name of the processor.
 * @throw	ProcessorNotFound	If the processor can not be found.
 * @ingroup dyn
 */
DynProcessor::DynProcessor(cstring name) throw(ProcessorNotFound) {
	proc = ProcessorPlugin::getProcessor(name);
	if(!proc)
		throw ProcessorNotFound(name);
}


/**
 */
DynProcessor::~DynProcessor(void) {
	if(proc)
		delete proc;
}


/**
 * @fn string DynProcessor::name(void) const;
 * Get the name of the processor.
 * @return	Processor name.
 */


/**
 * @fn Version DynProcessor::version(void) const;
 * Get the version of the processor.
 * @return	Processor version.
 */


/**
 * @fn void DynProcessor::configure(const PropList &props);
 * Configure the processor.
 * @param props	Configuration to pass to the processor.
 */

/**
 * Process the given workspace on the processor.
 * @param ws	Workspace to work on.
 * @param props	Configuration properties.
 */
void DynProcessor::process(WorkSpace *ws, const PropList &props) {
	ASSERTP(proc, "an unresolved dynamic processor must not be used !");
	proc->process(ws, props);
}

}	// otawa
