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
#include <elm/xom/Serializer.h>
#include <otawa/cfgio/Output.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/ipet/features.h>


namespace otawa { namespace cfgio {

using namespace elm;

p::declare Output::reg = p::init("otawa::cfgio::Output", Version(1, 0, 0))
	.maker<Output>()
	.base(BBProcessor::reg);


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
 * Output the current CFG collection in XML matching the DTA ${OTAWA_HOME}/share/Otawa/dtd/cfg.dtd .
 * @ingroup cfgio
 */

/**
 */
Output::Output(void): BBProcessor(reg), root(0), cfg_node(0), last_bb(0) {
}


/**
 * Generate ID for a CFG.
 * @param cfg	CFG to generate ID for.
 * @return		ID of the CFG.
 */
string Output::id(CFG *cfg) {
	return _ << '_' << cfg->number();
}


/**
 * Generate ID for a BB.
 * @param bb	BB to generate ID for.
 * @return		ID for the BB.
 */
string Output::id(BasicBlock *bb) {
	string suff;
	/*if(bb->isEntry())
		return _ << '_' << bb->cfg()->number() << "-entry";
	else if(bb->isExit())
		return _ << '_' << bb->cfg()->number() << "-exit";
	else*/
	return _ << '_' << bb->cfg()->number() << '-' << bb->number();
}


/**
 */
void Output::processCFG(WorkSpace *ws, CFG *cfg) {

	// build the CFG node
	cfg_node = new xom::Element("cfg");
	root->appendChild(cfg_node);
	string addr = id(cfg);
	string label = cfg->label();
	string num = _ << cfg->number();
	cfg_node->addAttribute(new xom::Attribute("id", &addr));
	cfg_node->addAttribute(new xom::Attribute("address", &addr));
	cfg_node->addAttribute(new xom::Attribute("label", &label));
	cfg_node->addAttribute(new xom::Attribute("number", &num));
	processProps(cfg_node, *cfg);

	// build the entry BB
	xom::Element *entry = new xom::Element("entry");
	cfg_node->appendChild(entry);
	string entry_id = id(cfg->entry());
	entry->addAttribute(new xom::Attribute("id", &entry_id));
	last_bb = cfg_node->getChildCount();

	// usual processing
	BBProcessor::processCFG(ws, cfg);

	// add the exit node
	xom::Element *exit = new xom::Element("exit");
	cfg_node->insertChild(exit, last_bb);
	string exit_id = id(cfg->exit());
	exit->addAttribute(new xom::Attribute("id", &exit_id));
}


static string replace(char c1, char c2, cstring s) {
	StringBuffer buf;
	for(int i = 0; i < s.length(); i++) {
		if(s[i] == c1)
			buf << c2;
		else
			buf << s[i];
	}
	return buf.toString();
}


/**
 */
void Output::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

	// add the basic
	if(!bb->isEnd()) {

		// make the BB element
		xom::Element *bb_node = new xom::Element("bb");
		string _id = id(bb);
		string addr = _ << bb->address();
		string size = _ << bb->size();
		string num = _ << bb->number();
		cfg_node->insertChild(bb_node, last_bb++);
		bb_node->addAttribute(new xom::Attribute("id", &_id));
		bb_node->addAttribute(new xom::Attribute("address", &addr));
		bb_node->addAttribute(new xom::Attribute("size", &size));
		bb_node->addAttribute(new xom::Attribute("number", &num));
		processProps(bb_node, *bb);

		// make the list of instruction
		for(BasicBlock::InstIter inst(bb); inst; inst++) {
			xom::Element *inst_node = new xom::Element("inst");
			bb_node->appendChild(inst_node);
			string addr = _ << inst->address();
			inst_node->addAttribute(new xom::Attribute("address", &addr));
			Option<Pair<cstring, int> > line_info = ws->process()->getSourceLine(inst->address());
			if(line_info) {
				string line = _ << (*line_info).snd;
				inst_node->addAttribute(new xom::Attribute("file", (*line_info).fst));
				inst_node->addAttribute(new xom::Attribute("line", &line));
			}
		}

		// if provided, gives the time

	}

	// add the output edges
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		xom::Element *edge_node = new xom::Element("edge");
		cfg_node->appendChild(edge_node);
		string kind = replace(' ', '-', Edge::kindName(edge->kind()));
		edge_node->addAttribute(new xom::Attribute("kind", &kind));
		string source = id(edge->source());
		edge_node->addAttribute(new xom::Attribute("source", &source));
		if(edge->kind() == Edge::CALL) {
			string called = id(edge->calledCFG());
			edge_node->addAttribute(new xom::Attribute("called", &called));
		}
		else {
			string target = id(edge->target());
			edge_node->addAttribute(new xom::Attribute("target", &target));
		}
		processProps(edge_node, **edge);
	}
}


/**
 */
void Output::processWorkSpace(WorkSpace *ws) {

	// build the root node
	root = new xom::Element("cfg-collection");

	// normal processing
	BBProcessor::processWorkSpace(ws);

	// output the XML
	xom::Document doc(root);
	xom::Serializer serial(io::out);
	serial.write(&doc);
	serial.flush();
}


/**
 * Output the properties.
 * @param parent	Parent element.
 * @param props		Properties to output.
 */
void Output::processProps(xom::Element *parent, PropList& props) {
	for(PropList::Iter prop(props); prop; prop++)
		if(prop->id()->name()) {
			xom::Element *prop_node = new xom::Element("property");
			parent->appendChild(prop_node);
			prop_node->addAttribute(new xom::Attribute("identifier", &prop->id()->name()));
			StringBuffer buf;
			prop->id()->print(buf, *prop);
			string s = buf.toString();
			prop_node->appendChild(&s);
		}
}


class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::cfgio", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }	// otawa::cfgio

otawa::cfgio::Plugin OTAWA_PROC_HOOK;
otawa::cfgio::Plugin& otawa_dcache = OTAWA_PROC_HOOK;
