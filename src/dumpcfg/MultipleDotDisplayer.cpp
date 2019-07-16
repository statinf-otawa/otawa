/*
 *	MultipleDotDisplayer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include "config.h"
#include "MultipleDotDisplayer.h"

#include <elm/sys/ProcessBuilder.h>
#include <elm/sys/System.h>
#include <elm/xom/String.h>
#include <otawa/cfg.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include <otawa/display/CFGOutput.h>

using namespace elm;
using namespace otawa;

/**
 */
MultipleDotDisplayer::MultipleDotDisplayer(void): Displayer("MultipleDotDisplayer", Version(1, 0, 0)) {
}

/**
 */
void MultipleDotDisplayer::setup(WorkSpace *ws) {

	// determine directory
	if(out)
		dir = out;
	else
		dir = ws->workDir() / "cfg";

	// build it or use it
	if(!dir.exists())
		dir.makeDirs();
}

/**
 */
void MultipleDotDisplayer::processWorkSpace(WorkSpace *ws) {
	PropList props;
	display::CFGOutput::PATH(props) = dir;
	display::CFGOutput::SOURCE(props) = source_info;
	display::CFGOutput::ASSEMBLY(props) = display_assembly;
	display::CFGOutput::SEM(props) = display_sem;
	display::CFGOutput::IKIND(props) = display_kind;
	display::CFGOutput::REGS(props) = display_regs;
	display::CFGOutput::TARGET(props) = display_target;
	display::CFGOutput::BYTES(props) = display_bytes;

	ws->run<display::CFGOutput>(props);

	// if requested, launch the viewer
	if(perform_view) {
#		if defined(XDOT_ENABLED) || defined(SYSTEM_VIEW_ENABLED)
#			ifdef XDOT_ENABLED
				sys::Path command = MANAGER.prefixPath() / "bin" / "otawa-xdot.py";
#			else
				sys::Path command = SYSTEM_VIEW;
#			endif
			sys::ProcessBuilder builder(command);
			builder += dir / "index.dot";
			builder.setNewSession(true);
			try {
				sys::Process *proc = builder.run();
				//proc->wait();
				delete proc;
			}
			catch(sys::SystemException& e) {
				cerr << "ERROR: cannot launch otawa-xdot: " << e << io::endl;
			}
#	else
		warn("neither otawa-xdot, nor system viewer are available to view the graph!");
#	endif

	}
}
