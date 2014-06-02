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
#include <otawa/display/GenDrawer.h>
#include <otawa/display/CFGAdapter.h>

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

// backlink to the CFGOutput
static Identifier<CFGOutput *> OUT("", 0);

// CFGOutputDecorator class
class CFGOutputDecorator {
public:
	static void decorate(const CFGAdapter &graph, Output &caption, TextStyle &text, FillStyle &fill) {
		CFGOutput *out = OUT(graph.cfg);
		ASSERT(out);
		out->genGraphLabel(graph.cfg, caption);
	}

	static void decorate(const CFGAdapter &graph, const CFGAdapter::Vertex vertex, Output &content, ShapeStyle &style) {
		style.shape = ShapeStyle::SHAPE_MRECORD;
		CFGOutput *out = OUT(graph.cfg);
		ASSERT(out);
		out->genBBLabel(graph.cfg, vertex.bb, content);
	}

	static void decorate(const CFGAdapter &graph, const CFGAdapter::Edge edge, Output &label, TextStyle &text, LineStyle &line) {
		switch(edge.edge->kind()){
		case otawa::Edge::VIRTUAL_CALL:
		case otawa::Edge::VIRTUAL:
		case otawa::Edge::VIRTUAL_RETURN:
			line.style = LineStyle::DASHED;
			break;
		}
		CFGOutput *out = OUT(graph.cfg);
		ASSERT(out);
		out->genEdgeLabel(graph.cfg, edge.edge, label);
	}
};

p::declare CFGOutput::reg =
	p::init("otawa::display::CFGOutput", Version(1, 1, 0), CFGProcessor::reg)
	.maker<CFGOutput>();

/**
 * Build the processor.
 */
CFGOutput::CFGOutput(AbstractRegistration& _reg): CFGProcessor(_reg), kind(OUTPUT_DOT) {

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
		label = _ << "cfg_" << cfg->label();
	Path out_path = path;
	out_path = out_path / label;
	out_path = out_path.setExtension(exts[kind]);

	// Perform the output
	if(logFor(LOG_PROC))
		cout << "\toutput " << label << " to " << out_path << io::endl;
	OUT(cfg) = this;
	CFGAdapter cfga(cfg);
	GenDrawer<CFGAdapter, CFGOutputDecorator> drawer(cfga);
	drawer.default_vertex.shape = ShapeStyle::SHAPE_MRECORD;
	drawer.default_vertex.text.size = 12;
	drawer.default_edge_text.size = 12;
	drawer.kind = kind;
	drawer.path = out_path.toString().toCString();
	drawer.draw();
	cfg->removeProp(OUT);
}


/**
 * Called to generate the label of the graph.
 * May be overload to customize the output.
 * @param cfg	Displayed CFG.
 * @param out	Output to generate the CFG label to.
 */
void CFGOutput::genGraphLabel(CFG *cfg, Output& out) {
	out << cfg->label() << " CFG";
}


/**
 * Called to generate the label of a basic block.
 * May be overload to customize the output.
 * In the end, this method call the genBBInfo() method.
 * To add a separation bar in genBBInfo(), frist output "---\n".
 * @param cfg	Displayed CFG.
 * @param bb	Displayed BB.
 * @param out	Output to generate the CFG label to.
 */
void CFGOutput::genBBLabel(CFG *cfg, BasicBlock *bb, Output& out) {

	// special of entry or exit
	if(bb->isEnd()) {
		if(bb->isEntry())
			out << "ENTRY";
		else if(bb->isExit())
			out << "EXIT";
		return;
	}

	// make title
	out << bb << "\n---\n";
	StringBuffer title;

	// make body
	for(BasicBlock::InstIter inst(bb); inst; inst++){

		// display labels
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
			out << *label << ":\n";
		for(Identifier<String>::Getter label(inst, otawa::LABEL); label; label++)
			out << *label << ":\n";

		// display the instruction
		out << "0x" << ot::address(inst->address()) << ":  ";
		inst->dump(out);
		out << "\n";
	}

	// give special format for Entry and Exit
	genBBInfo(cfg, bb, out);
}


/**
 * Called to generate the label of a basic block.
 * May be overload to customize the output.
 * In the end, this method call the genBBInfo() method.
 * @param cfg	Displayed CFG.
 * @param edge	Displayed edge.
 * @param out	Output to generate the CFG label to.
 */
void CFGOutput::genEdgeLabel(CFG *cfg, otawa::Edge *edge, Output& out) {
	switch(edge->kind()){
	case otawa::Edge::CALL: 			out << "call";		break;
	case otawa::Edge::TAKEN:			out << "taken";		break;
	case otawa::Edge::NOT_TAKEN:							break;
	case otawa::Edge::VIRTUAL:			out << "virtual";	break;
	case otawa::Edge::VIRTUAL_CALL:		out << "call";		break;
	case otawa::Edge::VIRTUAL_RETURN:	out << "return";	break;
	default:							ASSERT(false);		break;
	}
	genEdgeInfo(cfg, edge, out);
}


/**
 * Generate the information part of a basic block (called after the list of instructions).
 * @param cfg	Displayed CFG.
 * @param bb	Displayed BB.
 * @param out	Output to generate the CFG label to.
 */
void CFGOutput::genBBInfo(CFG *cfg, BasicBlock *bb, Output& out) {
	out << "---\n";
	for(PropList::Iter prop(bb); prop; prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, prop);
		out << io::endl;
	}
}


/**
 * Generate the information part of an edge (called after the label).
 * @param cfg	Displayed CFG.
 * @param edge	Displayed edge.
 * @param out	Output to generate the CFG label to.
 */
void CFGOutput::genEdgeInfo(CFG *cfg, otawa::Edge *edge, Output& out) {
	out << "\n";
	for(PropList::Iter prop(edge); prop; prop++) {
		out << prop->id()->name() << " = ";
		prop->id()->print(out, prop);
		out << io::endl;
	}
}

} } // otawa::display
