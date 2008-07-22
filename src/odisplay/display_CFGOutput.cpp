/*
 *	$Id$
 *	CFGOutput class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#include <otawa/display/CFGOutput.h>
#include <otawa/proc/Processor.h>
#include <elm/system/Path.h>
#include <otawa/display/CFGDrawer.h>

namespace otawa { namespace display {

using namespace display;

/**
 * @class CFGOutput
 * @author H. Cassé <casse@irit.fr>
 * This processor produces an output of the CFG of the current workspace
 * (including disassembly and attribute dump). For each CFG involved in the
 * computation, a file CFG_NAME.EXT is created with the EXT, extension, depends
 * on the @ref CFGOutput::KIND configuration.
 * 
 * @par Configuration
 * @li @ref CFGOutput::KIND
 * @li @ref CFGOutput::PATH
 * 
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 */


// Data
cstring exts[] = {
	"",
	"ps",
	"pdf",
	"png",
	"gif",
	"jpg",
	"svg",
	"dot"
};


/**
 * Build the processor.
 */
CFGOutput::CFGOutput(void)
:	CFGProcessor("otawa::CFGOutput", Version(1, 0, 0)) {
}


/**
 * Configuration identifier of @ref CFGOutput for the kind of generated file.
 */
Identifier<display::kind_t> CFGOutput::KIND("otawa::CFGOutput::KIND", OUTPUT_PS);


/**
 * Configuration identifier of @ref CFGOutput for the directory path where to
 * create the output file.
 */
Identifier<string> CFGOutput::PATH("otawa::CFGOutput::PATH", ".");


/**
 */
void CFGOutput::configure(const PropList &props) {
	CFGProcessor::configure(props);
	kind = KIND(props);
	path = PATH(props);
}


/**
 */
void CFGOutput::processCFG(WorkSpace *fw, CFG *cfg) {
	ASSERT(fw);
	ASSERT(cfg);
	
	// Compute the name
	string label = cfg->label();
	if(label == "")
		label = _ << "cfg_" << cfg->address();
	Path out_path = path;
	out_path = out_path / label;
	out_path = out_path.setExtension(exts[kind]);
	
	// Perform the output
	PropList props;
	if(isVerbose())
		cout << "\toutput " << label << " to " << out_path << io::endl;
	display::OUTPUT_PATH(props) = out_path.toString().toCString();
	display::OUTPUT_KIND(props) = kind;
	display::CFGDrawer drawer(cfg, props);
	drawer.display();
}

} } // otawa::display
