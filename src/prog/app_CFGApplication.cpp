/*
 *	CFGApplication class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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

#include <otawa/prog/Process.h>
#include <otawa/app/CFGApplication.h>
#include <otawa/view/features.h>

namespace otawa {

/**
 * @class CFGApplication
 *
 * A CFG application in OTAWA is used to define an application working on CFGs.
 * In addition to call the function processTask() for each function name
 * passed on the command line, it is able to prepare the CFG according
 * to the current architecture. Indeed, to make easier, faster, feasible or
 * more efficient the analyzes, OTAWA can transform the CFGs to take into
 * account some architecture features like delayed branches, conditional
 * instruction and so on.
 *
 * Moreover, this application provides some option that are proper to CFG:
 *	* --raw-cfg -- do not perform any CFG transformation
 *	* --virtualize -- replace each call to a function by a duplication of its CFG
 *		(improve the analysis precision but increase also the computation time).
 *	* --unroll-loops -- peel out the first iteration of each loop.
 *
 * @ingroup application
 */


////
CFGApplication::CFGApplication(const Make& make)
	:	Application(make),
		cfg_raw(make_switch().cmd("--cfg-raw").help("do not perform any CFG transformation to support architecture features")),
		cfg_virtualize(make_switch().cmd("--cfg-virtualize").help("duplicate called CFG according to their call sites")),
		cfg_unroll(make_switch().cmd("--cfg-unroll").help("unroll the first iteration of each loop")),
		no_cfg_tune(option::SwitchOption::Make(*this).cmd("--cfg-no-tune").description("disable tuning of CFGs for more user friendly work"))
{ }


/**
 * This function is called for each task entry name found in the command line.
 * The given CFG collection corresponds to the current task.
 * The default implementation does nothing.
 * @param coll	Current CFG collection.
 * @param props	Current property to configure invoked features.
 */
void CFGApplication::processTask(const CFGCollection& coll, PropList& props) {
}


///
void CFGApplication::work(const string& entry, PropList &props) {
	prepareCFG(entry, props);
	processTask(*COLLECTED_CFG_FEATURE.get(workspace()), props);
	workspace()->invalidate(COLLECTED_CFG_FEATURE);
}

/**
 * Apply the CFG options.
 * @param entry		Entry function name.
 * @param props		Current configuration.
 */
void CFGApplication::prepareCFG(const string& entry, PropList& props) {

	// tune the CFGs
	if(!no_cfg_tune) {
		Inst *i = workspace()->process()->findInstAt("__stack_chk_fail");
		if(i != nullptr)
			otawa::NO_CALL(i) = true;
	}

	// prepare the CFGs
	if(!cfg_raw) {
		if(workspace()->isProvided(DELAYED_FEATURE))
			require(DELAYED_CFG_FEATURE);
		if(workspace()->isProvided(CONDITIONAL_INSTRUCTIONS_FEATURE))
			require(CONDITIONAL_RESTRUCTURED_FEATURE);
	}
	require(otawa::REDUCED_LOOPS_FEATURE);
	if(cfg_unroll)
		require(UNROLLED_LOOPS_FEATURE);
	if(cfg_virtualize)
		require(VIRTUALIZED_CFG_FEATURE);

	// call the task processing
	require(COLLECTED_CFG_FEATURE);
}

} // otawa
