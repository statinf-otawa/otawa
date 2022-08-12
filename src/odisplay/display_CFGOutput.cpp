/*
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
 */
class ViewDecorator: public CFGDecorator {
public:

	ViewDecorator(WorkSpace *ws, view::View *view): CFGDecorator(ws),  _view(view) {
	}

	void displayBody(CFG *graph, BasicBlock *block, Text& content) const override {
		//for(_viewer->start(block); *_viewer; (*_viewer)++)
		//	_viewer->print(content);
	}

private:
	view::View *_view;
};


/**
 * @class CFGOutput
 * @author H. Cassé <casse@irit.fr>
 * This processor produces an output of the CFGs of the current workspace
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


// CFGOutputDecorator class
class CFGOutputDecorator: public CFGDecorator {
public:
	inline CFGOutputDecorator(CFGOutput& out, WorkSpace *ws, bool raw = false)
	:	CFGDecorator(ws),
		_out(out),
		rawInfo(raw),
		pf(*ws->process()->platform())
	{ }

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

	void displayAssembly(CFG *graph, BasicBlock *block, Text& content) const override {
		if(_out._ass)
			CFGDecorator::displayAssembly(graph, block, content);
	}

	void displayInst(Inst *i, Text& content) const override {
		if(_out._bytes) {
			content << begin(Color("grey"));
			for(auto a = i->address(); a < i->topAddress(); a = a + 1) {
				t::uint8 b;
				ws->process()->get(a, b);
				content << hex(b).width(2).pad('0');
			}
			content << end(Color("grey")) << ' ';
		}
		CFGDecorator::displayInst(i, content);
	}

	void displayInfo(Inst *i, Text& content) const override {
		if(_out._ikind)
			content << display::begin(display::SMALL) << display::begin(_out._sem_color)
					<< display::indent(4) << "kind = " << i->getKind()
					<< display::end(_out._sem_color) << display::end(display::SMALL) << display::left;
		if(_out._target && i->isControl() && !i->isReturn() && !i->isSpecial())
			content << display::begin(display::SMALL) << display::begin(_out._sem_color)
					<< display::indent(4) << "target = " << i->target()->address()
					<< display::end(_out._sem_color) << display::end(display::SMALL) << display::left;
		if(_out._regs) {
			RegSet rs, ws;
			i->readRegSet(rs);
			i->writeRegSet(ws);
			if(rs || ws) {
				content << display::begin(display::SMALL) << display::begin(_out._sem_color);
				if(rs) {
					content << display::indent(4) << "read regs: ";
					content << pf.findReg(rs[0])->name();
					for(int i = 1; i < rs.length(); i++)
						content << ", " << pf.findReg(rs[i])->name();
					content << display::left;
				}
				if(ws) {
					content << display::indent(4) << "written regs: ";
					content << pf.findReg(ws[0])->name();
					for(int i = 1; i < ws.length(); i++)
						content << ", " << pf.findReg(ws[i])->name();
					content << display::left;
				}
				content << display::end(_out._sem_color) << display::end(display::SMALL);
			}
		}
		if(_out._sem) {
			sem::Block b;
			i->semInsts(b);
			if(b) {
				content << display::begin(display::SMALL) << display::begin(_out._sem_color);
				for(auto si: b)
					content << display::indent(4) << si << display::left;
				content << display::end(_out._sem_color) << display::end(display::SMALL);
			}
			i->semInsts(b);
		}
	}

	virtual void displaySynthBlock(CFG *g, SynthBlock *b, Text& content, VertexStyle& style) const {
		display::CFGDecorator::displaySynthBlock(g, b, content, style);
		if(b->callee()) {
			if(!b->callee()->index())
				content.setURL("index.dot");
			else
				content.setURL(_ << b->callee()->index() << ".dot");
		}
	}

private:
	CFGOutput& _out;
	bool rawInfo;
	hard::Platform& pf;
};

p::declare CFGOutput::reg =
	p::init("otawa::display::CFGOutput", Version(2, 1, 0), CFGProcessor::reg)
	.maker<CFGOutput>();

/**
 * Build the processor.
 */
CFGOutput::CFGOutput(AbstractRegistration& _reg):
	CFGProcessor(_reg),
	kind(OUTPUT_DOT),
	rawInfo(false),
	dec(nullptr),
	_view(nullptr),
	_sem(false),
	_source(true),
	_props(false),
	_ass(true),
	_ikind(false),
	_regs(false),
	_target(false),
	_bytes(false),
	_sem_color("blue"),
	_source_color("green")
{
}


/**
 * Configuration identifier of @ref CFGOutput for the kind of generated file.
 */
p::id<display::kind_t> CFGOutput::KIND("otawa::display::CFGOutput::KIND", OUTPUT_DOT);

/**
 * Configuration identifier of @ref CFGOutput for the directory path where to
 * create the output file.
 */
p::id<string> CFGOutput::PATH("otawa::display::CFGOutput::PATH");

/**
 * Configuration identifier of @ref CFGOutput for the prefix file name
 */
p::id<string> CFGOutput::PREFIX("otawa::display::CFGOutput::PREFIX", "");

/**
 * Configuration identifier of @ref CFGOutput to decide if the blocks will be using the raw information from the CFGOutput::genBBLabel()
 */
p::id<bool> CFGOutput::RAW_BLOCK_INFO("otawa::display::CFGOutput::RAW_BLOCK_INFO", false);

/**
 * Configuration identifier of @ref CFGOutput to decide which view to use.
 */
p::id<view::View *> CFGOutput::VIEW("otawa::display::CFGOutput::VIEW", nullptr);

/**
 * Configuration identifier of CFGOutput to enable display of semantics instructions.
 */
p::id<bool> CFGOutput::SEM("otawa::display::CFGOutput::SEM", false);

/**
 * Configuration identifier of CFGOutput to enable display of source lines.
 */
p::id<bool> CFGOutput::SOURCE("otawa::display::CFGOutput::SOURCE", false);

/**
 * Configuration identifier of CFGOutput to enable display of properties.
 */
p::id<bool> CFGOutput::PROPS("otawa::display::CFGOutput::PROPS", false);

/**
 * Configuration identifier of CFGOutput to enable display of assembly.
 */
p::id<bool> CFGOutput::ASSEMBLY("otawa::display::CFGOutput::ASSEMBLY", false);

/**
 * Configuration identifier of CFGOutput to enable display of instruction kind.
 */
p::id<bool> CFGOutput::IKIND("otawa::display::CFGOutput::IKIND", false);

/**
 * Configuration identifier of CFGOutput to enable display of used registers.
 */
p::id<bool> CFGOutput::REGS("otawa::display::CFGOutput::REGS", false);

/**
 * Configuration identifier of CFGOutput to enable display of branch instruction target.
 */
p::id<bool> CFGOutput::TARGET("otawa::display::CFGOutput::TARGET", false);

/**
 * Configuration identifier of CFGOutput to enable display of instruction bytes.
 */
p::id<bool> CFGOutput::BYTES("otawa::display::CFGOutput::BYTES", false);

/**
 * Configuration identifier of CFGOutput to select colors of semantic instructions.
 */
p::id<Color> CFGOutput::SEM_COLOR("otawa::display::CFGOutput::SEM_COLOR");

/**
 * Configuration identifier of CFGOutput to select colors of source line.
 */
p::id<Color> CFGOutput::SOURCE_COLOR("otawa::display::CFGOutput::SOURCE_COLOR");


/**
 */
void CFGOutput::configure(const PropList &props) {
	CFGProcessor::configure(props);
	kind = KIND(props);
	path = PATH(props);
	rawInfo = RAW_BLOCK_INFO(props);
	_view = VIEW(props);
	_sem = SEM(props);
	_source = SOURCE(props);
	_props = PROPS(props);
	_ass = ASSEMBLY(props);
	_ikind = IKIND(props);
	_regs = REGS(props);
	_target = TARGET(props);
	_bytes = BYTES(props);
	Color c = SEM_COLOR(props);
	if(c)
		_sem_color = c;
	c = SOURCE_COLOR(props);
	if(c)
		_source_color = c;
}

/**
 */
void CFGOutput::setup(WorkSpace *ws) {

	// TODO Improve it. Remove the full directory before re-creating it!

	// get a valid path
	if(path.isEmpty())
		path = sys::Path(ws->process()->program()->name()).setExtension("cfg");

	// remove existing one
	if(path.exists()) {
		if(!path.isDir())
			throw ProcessorException(*this, _ << "cannot create the directory " << path << ": a file exists with same name" );
	}

	// create it
	else {
		try {
			if(logFor(LOG_PROC))
				log << "\tcreating directory" << path << io::endl;
			sys::System::makeDir(path);
		}
		catch(sys::SystemException& e) {
			throw ProcessorException(*this, _ << "error during creation of " << path << ": " << e.message());
		}
	}

	// verbose output
	if(logFor(LOG_PROC))
		log << "\toutput directory = " << path << io::endl;
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

	// prepare the decorator
	if(_view)
		dec = new ViewDecorator(ws, _view);
	else
		dec = new CFGOutputDecorator(*this, ws);
	dec->display_assembly = _ass;
	dec->display_source_line = _source;
	dec->display_props = rawInfo;

	// perform the output
	display::Displayer *disp = display::Provider::display(g, *dec, kind);
	disp->setPath(opath);
	if(logFor(LOG_PROC))
		log << "\toutput CFG " << g << " to " << disp->path() << io::endl;
	disp->process();

	// release the decorator
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
	dec->CFGDecorator::decorate(cfg, caption, style);
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
	dec->CFGDecorator::decorate(cfg, block, content, style);
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
	dec->CFGDecorator::decorate(cfg, edge, label, style);
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
	for(PropList::Iter prop(block); prop(); prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, *prop);
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
	for(PropList::Iter prop(edge); prop(); prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, *prop);
		out << io::endl;
	}
}

} } // otawa::display
