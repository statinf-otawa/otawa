/*
 *	cfgio::Output class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#include <elm/io/OutFileStream.h>
#include <elm/xom/Serializer.h>

#include <elm/avl/Set.h>
#include <elm/xom.h>
#include <otawa/cfg.h>
#include <otawa/cfgio/features.h>
#include <otawa/proc/BBProcessor.h>

#include <otawa/data/clp/features.h>

#include <otawa/ipet/features.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prog/Process.h>
#include <otawa/prop/DynIdentifier.h>



namespace otawa { namespace clp {

using namespace elm;

class CLPExport: public BBProcessor {
public:
	static p::declare reg;
	CLPExport(void);
protected:
	virtual void configure(const PropList& props);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b);
	string id(CFG *cfg);
	string id(Block *bb);
	void processProps(xom::Element *parent, const PropList& props);
	void processState(xom::Element *parent, const clp::State& clpState);
	xom::Element *root, *cfg_node;
	int last_bb;
	Path path;
};

p::declare CLPExport::reg = p::init("otawa::clp::CLPExport", Version(1, 0, 0))
	.maker<CLPExport>()
	.base(BBProcessor::reg);


/**
 * Used to select the file path to output to.
 * If not defined, output is performed to standard output.
 */
Identifier<Path> EXPORT_PATH("otawa::clp::EXPORT_PATH", "pqr");


/**
 * @defgroup cfgio	CFG Input / Output
 *
 * This plugin is responsible to perform input / output of the CFG collection.
 * This allows external work on the CFG provided by OTAWA. As a default, XML is provided
 * with files matching the DTD from ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 *
 * You can use this module in a script or embed it in an executable using the following
 * @c otawa-config result:
 * @code
 * 	otawa-config --libs --rpath cfgio
 * @endcode
 */

/**
 * @class Output
 * Output the current CFG collection in XML matching the DTD ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 *
 * @par Configuration
 * @li @ref INCLUDE -- include the identifier whose name is given in the output.
 * @li @ref OUTPUT -- path to output file to (if not defined, output to standard output).
 * @ingroup cfgio
 */

/**
 */
CLPExport::CLPExport(void): BBProcessor(reg), root(0), cfg_node(0), last_bb(0) {
}


/**
 * Generate ID for a CFG.
 * @param cfg	CFG to generate ID for.
 * @return		ID of the CFG.
 */
string CLPExport::id(CFG *cfg) {
	return _ << '_' << cfg->index();
}


/**
 * Generate ID for a BB.
 * @param bb	BB to generate ID for.
 * @return		ID for the BB.
 */
string CLPExport::id(Block *bb) {
	string suff;
	return _ << '_' << bb->cfg()->index() << '-' << bb->index();
}


/**
 */
void CLPExport::processCFG(WorkSpace *ws, CFG *cfg) {

	// build the CFG node
	cfg_node = new xom::Element("cfg");
	root->appendChild(cfg_node);
	string addr = _ << "0x" << cfg->address();
	string label = cfg->label();
	string num = _ << cfg->index();
	string _id = id(cfg);
	cfg_node->addAttribute(new xom::Attribute("id", _id.asNullTerminated()));
	cfg_node->addAttribute(new xom::Attribute("address", addr.asNullTerminated()));
	cfg_node->addAttribute(new xom::Attribute("label", label.asNullTerminated()));
	cfg_node->addAttribute(new xom::Attribute("number", num.asNullTerminated()));
	//processProps(cfg_node, *cfg);

	// build the entry BB
	xom::Element *entry = new xom::Element("entry");
	cfg_node->appendChild(entry);
	string entry_id = id(cfg->entry());
	entry->addAttribute(new xom::Attribute("id", entry_id.asNullTerminated()));
	last_bb = cfg_node->getChildCount();

	// usual processing
	BBProcessor::processCFG(ws, cfg);

	// add the exit node
	xom::Element *exit = new xom::Element("exit");
	cfg_node->insertChild(exit, last_bb);
	string exit_id = id(cfg->exit());
	exit->addAttribute(new xom::Attribute("id", exit_id.asNullTerminated()));
}


/**
 */
void CLPExport::processBB(WorkSpace *ws, CFG *cfg, Block *b) {

	// add the basic
	if(!b->isEnd()) {

		// make the BB element
		xom::Element *bb_node = new xom::Element("bb");
		string _id = id(b);
		string num = _ << b->index();
		cfg_node->insertChild(bb_node, last_bb++);
		bb_node->addAttribute(new xom::Attribute("id", _id.asNullTerminated()));
		bb_node->addAttribute(new xom::Attribute("number", num.asNullTerminated()));

		// basic block specialization
		if(b->isBasic()) {
			BasicBlock *bb = b->toBasic();
			string addr = _ << "0x" << bb->address();
			string size = _ << bb->size();
			bb_node->addAttribute(new xom::Attribute("address", addr.asNullTerminated()));
			bb_node->addAttribute(new xom::Attribute("size", size.asNullTerminated()));

			// now for each instruction
			for(BasicBlock::InstIter bbii(bb); bbii(); bbii++) {
				xom::Element *instNode = new xom::Element("inst");
				bb_node->appendChild(instNode);
				string addr = _ << "0x" << bbii->address();
				string size = _ << bbii->size();
				instNode->addAttribute(new xom::Attribute("address", addr.asNullTerminated()));
				instNode->addAttribute(new xom::Attribute("size", size.asNullTerminated()));
				processProps(instNode, **bbii);
			}


		}
		else if(b->isSynth()) {
			CFG *cfg = b->toSynth()->callee();
			bb_node->addAttribute(new xom::Attribute("call", !cfg ? "" : id(cfg).asNullTerminated()));
		}
		processProps(bb_node, *b);
	}

}


/**
 */
void CLPExport::configure(const PropList& props) {
	BBProcessor::configure(props);
	path = clp::EXPORT_PATH(props);
}


/**
 */
void CLPExport::processWorkSpace(WorkSpace *ws) {

	// build the root node
	root = new xom::Element("clp-state-collection");

	// normal processing
	BBProcessor::processWorkSpace(ws);

	// open output
	io::OutFileStream *file = 0;
	io::OutStream *out = &io::out;
	if(path) {
		file = new OutFileStream(path);
		if(!file->isReady()) {
			cstring msg = file->lastErrorMessage();
			delete file;
			throw ProcessorException(*this, _ << "cannot open \"" << path << "\": " << msg);
		}
		out = file;
	}

	// output the XML
	xom::Document doc(root);
	xom::Serializer serial(*out);
	serial.write(&doc);
	serial.flush();

	// close file if needed
	if(file)
		delete file;
}


/**
 * Output the properties.
 * @param parent	Parent element.
 * @param props		Properties to output.
 */
void CLPExport::processState(xom::Element *parent, const clp::State& clpState) {
	if(clpState == clp::State::FULL)
		parent->addAttribute(new xom::Attribute("top", "true" ));
	else
		parent->addAttribute(new xom::Attribute("top", "false" ));

	for(clp::State::Iter clpsi(clpState); clpsi(); clpsi++) {
		clp::Value actualValue;
		if(clpsi.id().kind() == clp::REG) {
			if(*clpsi != clp::Value::top) {
				xom::Element *nodeReg = new xom::Element("reg");
				StringBuffer buf;
				buf << clpsi.id().lower();
				String str = buf.toString();
				nodeReg->addAttribute(new xom::Attribute("id", str.asNullTerminated()));
				parent->appendChild(nodeReg);

				buf.reset();
				buf << "0x" << hex((*clpsi).lower());
				str = buf.toString();
				nodeReg->addAttribute(new xom::Attribute("base", str.asNullTerminated()));

				buf.reset();
				buf << "0x" << hex((*clpsi).delta());
				str = buf.toString();
				nodeReg->addAttribute(new xom::Attribute("delta", str.asNullTerminated()));

				buf.reset();
				buf << "0x" << hex((*clpsi).mtimes());
				str = buf.toString();
				nodeReg->addAttribute(new xom::Attribute("mtimes", str.asNullTerminated()));
			}
		}
		else { // memory
			if(*clpsi != clp::Value::top) {
				xom::Element *nodeMem = new xom::Element("mem");
				StringBuffer buf;
				buf << clpsi.id().lower();
				String str = buf.toString();
				nodeMem->addAttribute(new xom::Attribute("addr", str.asNullTerminated()));
				parent->appendChild(nodeMem);

				buf.reset();
				buf << "0x" << hex((*clpsi).lower());
				str = buf.toString();
				nodeMem->addAttribute(new xom::Attribute("base", str.asNullTerminated()));

				buf.reset();
				buf << "0x" << hex((*clpsi).delta());
				str = buf.toString();
				nodeMem->addAttribute(new xom::Attribute("delta", str.asNullTerminated()));

				buf.reset();
				buf << "0x" << hex((*clpsi).mtimes());
				str = buf.toString();
				nodeMem->addAttribute(new xom::Attribute("mtimes", str.asNullTerminated()));
			}
		}
	}
}

void CLPExport::processProps(xom::Element *parent, const PropList& props) {
	xom::Element *nodeCLPStateIn = new xom::Element("clp_state_in");
	parent->appendChild(nodeCLPStateIn);
	clp::State clpState = clp::STATE_IN(props);
	processState(nodeCLPStateIn, clpState);

	xom::Element *nodeCLPStateOut = new xom::Element("clp_state_out");
	parent->appendChild(nodeCLPStateOut);
	clpState = clp::STATE_OUT(props);
	processState(nodeCLPStateOut, clpState);
}


} }	// otawa::clp


