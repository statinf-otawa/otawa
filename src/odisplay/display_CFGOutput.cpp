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

#include <elm/sys/Path.h>
#include <elm/sys/System.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/proc/Processor.h>
#include <otawa/program.h>

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
/*cstring exts[] = {
	"",
	"ps",
	"pdf",
	"png",
	"gif",
	"jpg",
	"svg",
	"dot",
	"raw"
};*/

// backlink to the CFGOutput
//static Identifier<CFGOutput *> OUT("", 0);

// CFGOutputDecorator class
class CFGOutputDecorator: public CFGDecorator {
public:
	inline CFGOutputDecorator(CFGOutput& out, WorkSpace *ws, bool raw = false): CFGDecorator(ws), _out(out), rawInfo(raw) { }

	void decorate(CFG *graph, Text& caption, GraphStyle& style) const override {
		_out.genGraphLabel(graph, caption, style);
	}

	void decorate(CFG *graph, Block *block, Text& content, VertexStyle& style) const override {
		style.shape = ShapeStyle::SHAPE_MRECORD;
		style.raw = rawInfo;
		_out.genBBLabel(graph, block, content, style);
	}

	void decorate(CFG *graph, otawa::Edge *edge, Text& label, EdgeStyle& style) const override {
		if(edge->sink()->isSynth()
		|| edge->source()->isEnd()
		|| edge->sink()->isEnd())
			style.line.style = LineStyle::DASHED;
		_out.genEdgeLabel(graph, edge, label, style);
	}

private:
	CFGOutput& _out;
	bool rawInfo;
};

p::declare CFGOutput::reg =
	p::init("otawa::display::CFGOutput", Version(1, 1, 0), CFGProcessor::reg)
	.maker<CFGOutput>();

/**
 * Build the processor.
 */
CFGOutput::CFGOutput(AbstractRegistration& _reg):
	CFGProcessor(_reg),
	kind(OUTPUT_DOT),
	rawInfo(false),
	dec(nullptr)
{
}


/**
 * Configuration identifier of @ref CFGOutput for the kind of generated file.
 */
Identifier<display::kind_t> CFGOutput::KIND("otawa::display::CFGOutput::KIND", OUTPUT_DOT);

/**
 * Configuration identifier of @ref CFGOutput for the directory path where to
 * create the output file.
 */
Identifier<string> CFGOutput::PATH("otawa::display::CFGOutput::PATH", ".");

/**
 * Configuration identifier of @ref CFGOutput for the prefix file name
 */
Identifier<string> CFGOutput::PREFIX("otawa::display::CFGOutput::PREFIX", "");

/**
 * Configuration identifier of @ref CFGOutput to decide if the blocks will be using the raw information from the CFGOutput::genBBLabel()
 */
Identifier<bool> CFGOutput::RAW_BLOCK_INFO("otawa::display::CFGOutput::RAW_BLOCK_INFO", false);

/**
 */
void CFGOutput::configure(const PropList &props) {
	CFGProcessor::configure(props);
	kind = KIND(props);
	path = PATH(props);
	rawInfo = RAW_BLOCK_INFO(props);
}

/**
 */
void CFGOutput::setup(WorkSpace *ws) {

	// TODO Improve it. Remove the full directory before re-creating it!

	// get a valid path
	if(path.isEmpty())
		path = sys::Path(ws->process()->program()->name()).setExtension(".cfg");

	// remove existing one
	if(path.exists()) {
		if(!path.isDir())
			throw ProcessorException(*this, _ << "cannot create the directory " << path << ": a file exists with same name" );
	}

	// create it
	else {
		try {
			if(logFor(LOG_PROC))
				cout << "\tcreating directory" << path << io::endl;
			sys::System::makeDir(path);
		}
		catch(sys::SystemException& e) {
			throw ProcessorException(*this, _ << "error during creation of " << path << ": " << e.message());
		}
	}
}

/**
 */
void CFGOutput::processCFG(WorkSpace *ws, CFG *g) {

	// compute the path
	sys::Path opath;
	if(g->index() == 0)
		opath = path / "index";
	else
		opath = path / string(_ << g->index());
	if(logFor(LOG_PROC))
		cout << "\toutput CFG " << g << " to " << opath << io::endl;

	// perform the output
	dec = new CFGOutputDecorator(*this, ws);
	display::Displayer *disp = display::Provider::display(g, *dec, kind);
	disp->setPath(path);
	disp->process();
	delete dec;
}


/**
 * Called to generate the label of the graph.
 * May be overload to customize the output.
 * @param cfg		Displayed CFG.
 * @param caption	To generate the caption of the graph.
 * @param style		Style for the graph (output).
 */
void CFGOutput::genGraphLabel(CFG *cfg, Text& caption, GraphStyle& style) {
	dec->decorate(cfg, caption, style);
}


/**
 * Called to generate the label of a basic block.
 * May be overload to customize the output.
 * In the end, this method call the genBBInfo() method.
 * To add a separation bar in genBBInfo(), frist output "---\n".
 * @param cfg		Displayed CFG.
 * @param block		Displayed Block.
 * @param content	To output the content of the block.
 * @param style		Style of the block (output).
 */
void CFGOutput::genBBLabel(CFG *cfg, Block *block, Text& content, VertexStyle& style) {
	dec->decorate(cfg, block, content, style);
	if(rawInfo)
		genBBInfo(cfg, block, content);
}


/**
 * Called to generate the label of a basic block.
 * May be overload to customize the output.
 * In the end, this method call the genBBInfo() method.
 * @param cfg	Displayed CFG.
 * @param edge	Displayed edge.
 * @param label	Output to generate the CFG label to.
 * @param style	Edge style (output).
 */
void CFGOutput::genEdgeLabel(CFG *cfg, otawa::Edge *edge, Text& label, EdgeStyle& style) {
	dec->decorate(cfg, edge, label, style);
	if(rawInfo)
		genEdgeInfo(cfg, edge, label);
}


/**
 * Generate the information part of a basic block (called after the list of instructions).
 * @param cfg		Displayed CFG.
 * @param block		Displayed BB.
 * @param content	To generate the BB information part.
 */
void CFGOutput::genBBInfo(CFG *graph, Block *block, Text& content) {
	out << "---\n";
	for(PropList::Iter prop(block); prop; prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, prop);
		out << io::endl;
	}
}


/**
 * Generate the information part of an edge (called after the label).
 * @param cfg	Displayed CFG.
 * @param edge	Displayed edge.
 * @param label	To generate the edge label.
 */
void CFGOutput::genEdgeInfo(CFG *graph, otawa::Edge *edge, Text& label) {
	out << "\n";
	for(PropList::Iter prop(edge); prop; prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, prop);
		out << io::endl;
	}
}

} } // otawa::display
