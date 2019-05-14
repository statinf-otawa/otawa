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
#include <otawa/ilp/System.h>
#include <otawa/ipet/features.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/Process.h>
#include <otawa/prop/DeletableProperty.h>

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


///
p::declare ILPSystemGetter::reg = p::init("otawa::ipet::ILPSystemGetter", Version(1, 1, 0))
	.make<ILPSystemGetter>()
	.provide(ILP_SYSTEM_FEATURE);


/**
 * Build the processor.
 */
ILPSystemGetter::ILPSystemGetter(void): Processor(reg), max(true) {
}


/**
 */
void ILPSystemGetter::processWorkSpace(WorkSpace *ws) {
	ASSERT(ws);
	ilp::System *sys = ws->process()->manager()->newILPSystem(plugin_name, max);
	if(logFor(LOG_DEPS)) {
		log << "\tmaking an ILP system from \""
			<< (plugin_name ? plugin_name : "default")
			<< "\" plugin\n";
		if(max)
			log << "\tfor maximization\n";
		else
			log << "\tfor minimization\n";
	}
	if(!sys)
		throw otawa::Exception("no ILP solver available !");
	SYSTEM(ws) = sys;
}


/**
 */
void ILPSystemGetter::configure(const PropList &props) {
	plugin_name = ILP_PLUGIN_NAME(props);
	Processor::configure(props);
	max = MAXIMIZE(props);
}


///
void ILPSystemGetter::destroy(WorkSpace *ws) {
	ilp::System *sys = SYSTEM(ws);
	ASSERT(sys);
	SYSTEM(ws).remove();
	delete sys;
}


/**
 * Select the name of the plugin to use as the ILP solver.
 * @par Processor Configuration
 * @li @ref ILPSystemGetter
 */
Identifier<string> ILP_PLUGIN_NAME("otawa::ipet::ILP_PLUGIN_NAME", "default");


/**
 * Link the curerently ILP system.
 *
 * @par Hooks
 *	* @ref WorkSpace
 *
 * @par Features
 *	* @ref ILP_SYSTEM_FEATURE
 *
 * @ingroup ipet
 */
Identifier<ilp::System *> SYSTEM("otawa::ipet::SYSTEM", 0);


/**
 * This feature assert that an ILP is available for IPET computation.
 *
 * @par Properties
 *	* @ref SYSTEM
 *
 * @par Processors
 *	* @ref ILPSystemGetter (default)
 *
 * @par Configuration
 *	* @ref MAXIMIZE
 *
 * @ingroup ipet
 */
p::feature ILP_SYSTEM_FEATURE("otawa::ipet::ILP_SYSTEM_FEATURE", new Maker<ILPSystemGetter>());

/**
 * This property is used to configure @ref ILP_SYSTEM_FEATURE. If set to true
 * (default), the ILP system will be maximized. Else it will be minimized.
 *
 * @par Features
 *	* @ref ILP_SYSTEM_FEATURE
 *
 * @ingroup ipet
 */
p::id<bool> MAXIMIZE("otawa::ipet::MAXIMIZE", true);

} } // otawa::ipet
