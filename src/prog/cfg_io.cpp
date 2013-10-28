/*
 *	I/O processor for the CFG module
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
#include <otawa/cfg/CFGSaver.h>
#include <elm/sys/System.h>
#include <elm/xom.h>
#include <elm/xom/Serializer.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>

namespace otawa {

static xom::String
	WORKSPACE_TAG = "otawa-workspace",
	CFG_TAG = "cfg",
	BB_TAG = "bb",
	EDGE_TAG = "edge",
	ENTRY_ATT = "entry",
	EXIT_ATT = "exit",
	ADDRESS_ATT = "address",
	SIZE_ATT = "size",
	SOURCE_ATT = "source",
	TARGET_ATT = "target",
	CFG_ATT = "cfg",
	ID_ATT = "id",
	KIND_ATT = "kind";

static inline string id(void *object) {
	return _ << object;
}

static inline void set(xom::Element *elt, xom::String att, const string& value) {
	elt->addAttribute(new xom::Attribute(att, &value));
}


/**
 * This property allows to configure the file used by @ref CFGLoader and @ref CFGSaver.
 *
 * @ingroup cfg
 */
Identifier<sys::Path> CFG_IO_PATH("otawa::CFG_IO_PATH");


/**
 * @class CFGSaver
 * Save the CFGs of the current task to a file.
 *
 * @param Configuration
 * @li @ref CFG_IO_PATH
 *
 * @ingroup cfg
 */

p::declare CFGSaver::reg = p::init("otawa::CFGSaver", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.config(CFG_IO_PATH)
	.maker<CFGSaver>();


/**
 */
CFGSaver::CFGSaver(p::declare& r): BBProcessor(reg), stream(&io::out), doc(0), root(0), cfg_elt(0) {
}

void CFGSaver::configure(const PropList& props) {
	path = CFG_IO_PATH(props);
}

void CFGSaver::setup(WorkSpace *ws) {
	root = new xom::Element(WORKSPACE_TAG);
	doc = new xom::Document(root);
}

void CFGSaver::processWorkSpace(WorkSpace *ws) {

	// create the document
	BBProcessor::processWorkSpace(ws);

	// save the document
	try {
		if(path)
			stream = sys::System::createFile(path);
		xom::Serializer serial(*stream);
		serial.write(doc);
		serial.flush();
	}
	catch(sys::SystemException& e) {
		throw ProcessorException(*this, _ << "cannot create \"" << path << "\": " + e.message());
	}
}

void CFGSaver::processCFG(WorkSpace *ws, CFG *cfg) {

	// build the element
	cfg_elt = new xom::Element(CFG_TAG);
	set(cfg_elt, ID_ATT, id(cfg));
	set(cfg_elt, ENTRY_ATT, id(cfg->entry()));
	set(cfg_elt, EXIT_ATT, id(cfg->exit()));
	root->appendChild(cfg_elt);

	// build the BBs
	BBProcessor::processCFG(ws, cfg);
}

void CFGSaver::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;

	// generate the BB element
	xom::Element *bb_elt = new xom::Element(BB_TAG);
	if(!bb->isEnd()) {
		set(bb_elt, ADDRESS_ATT, _ << bb->address());
		set(bb_elt, SIZE_ATT, _ << bb->size());
	}
	cfg_elt->appendChild(bb_elt);

	// generate the outing edges
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		if(edge->kind() != Edge::CALL && edge->target()->isExit())
			continue;
		xom::Element *edge_elt = new xom::Element(EDGE_TAG);
		set(edge_elt, KIND_ATT, Edge::kindName(edge->kind()));
		set(edge_elt, SOURCE_ATT, id(edge->source()));
		if(edge->kind() != Edge::CALL)
			set(edge_elt, TARGET_ATT, id(edge->target()));
		else
			set(edge_elt, CFG_ATT, id(edge->calledCFG()));
		cfg_elt->appendChild(edge_elt);
	}
}

void CFGSaver::cleanup(WorkSpace *ws) {
	if(stream != &io::out)
		delete stream;
	delete doc;
}

}	// otawa

