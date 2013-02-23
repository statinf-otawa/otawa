/*
 *	$Id$
 *	ILPSystemGetter class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#include <otawa/prog/Manager.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/ilp/System.h>
#include <otawa/prop/DeletableProperty.h>
#include <otawa/ipet/features.h>

namespace otawa { namespace ipet {

/**
 * @class ILPSystemGetter;
 * This processor looks for an ILP plugin and build a new system that will be
 * used by other IPET processor.
 *
 * @par Provided Features
 * @li @ref ILP_SYSTEM_FEATURE
 *
 * @par Configuration
 * @li @ref ILP_PLUGIN_NAME
 */


/**
 * Build the processor.
 */
ILPSystemGetter::ILPSystemGetter(void)
: Processor("otawa::ipet::ILPSystemGetter", Version(1, 0, 0)) {
	provide(ILP_SYSTEM_FEATURE);
}


/**
 */
void ILPSystemGetter::processWorkSpace(WorkSpace *ws) {
	ASSERT(ws);
	ilp::System *sys = ws->process()->manager()->newILPSystem(plugin_name);
	if(logFor(LOG_DEPS))
		log << "\tmaking an ILP system from \""
			<< (plugin_name ? plugin_name : "default")
			<< "\" plugin\n";
	if(!sys)
		throw otawa::Exception("no ILP solver available !");
	//ws->addDeletable(SYSTEM, sys);
	ws->addProp(new DeletableProperty<ilp::System *>(SYSTEM, sys));
}


/**
 */
void ILPSystemGetter::configure(const PropList &props) {
	plugin_name = ILP_PLUGIN_NAME(props);
	Processor::configure(props);
}


/**
 * Select the name of the plugin to use as the ILP solver.
 * @par Processor Configuration
 * @li @ref ILPSystemGetter
 */
Identifier<cstring> ILP_PLUGIN_NAME("otawa::ipet::ILP_PLUGIN_NAME", "");


/**
 * Link the curerently ILP system.
 * @par Hooks
 * @li @ref WorkSpace
 * @par Features
 * @li @ref ILP_SYSTEM_FEATURE
 */
Identifier<ilp::System *> SYSTEM("otawa::ipet::SYSTEM", 0);


static SilentFeature::Maker<ILPSystemGetter> maker;
/**
 * This feature assert that an ILP is available for IPET computation.
 * @par Properties
 * @li @ref SYSTEM
 * @par Default Processor
 * @ref @ref ILPSystemGetter
 */
SilentFeature ILP_SYSTEM_FEATURE("otawa::ipet::ILP_SYSTEM_FEATURE", maker);

} } // otawa::ipet
