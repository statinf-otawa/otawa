/*
 *	$Id$
 *	CFGOutput class interface
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
#ifndef OTAWA_CFGOUTPUT_H_
#define OTAWA_CFGOUTPUT_H_

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/view/features.h>
#include "CFGDecorator.h"

namespace otawa {

class Edge;

namespace display {

class CFGOutputDecorator;

// CFGOutput class
class CFGOutput: public CFGProcessor {
	friend class CFGOutputDecorator;
public:
	static p::declare reg;
	CFGOutput(AbstractRegistration& _reg = reg);

	// Configuration
	static p::id<display::kind_t> KIND;
	static p::id<string> PATH;
	static p::id<string> PREFIX;
	static p::id<bool> RAW_BLOCK_INFO;
	static p::id<view::View *> VIEW;
	static p::id<bool> SEM, SOURCE, PROPS, ASSEMBLY, IKIND, REGS, TARGET, BYTES;
	static p::id<Color> SEM_COLOR, SOURCE_COLOR;

	virtual void genGraphLabel(CFG *graph, Text& caption, GraphStyle& style);
	virtual void genBBLabel(CFG *graph, Block *block, Text& content, VertexStyle& style);
	virtual void genEdgeLabel(CFG *graph, otawa::Edge *edge, Text& label, EdgeStyle& style);
	virtual void genBBInfo(CFG *graph, Block *block, Text& content);
	virtual void genEdgeInfo(CFG *graph, otawa::Edge *edge, Text& label);

protected:
	virtual void configure(const PropList &props);
	virtual void setup(WorkSpace *ws);
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual string getMisc(void) { return ""; }

private:
	display::kind_t kind;
	sys::Path path;
	bool rawInfo;
	display::CFGDecorator *dec;
	view::View *_view;
	bool _sem, _source, _props, _ass, _ikind, _regs, _target, _bytes;
	Color _sem_color, _source_color;
};

} }	// otawa::display

#endif /* OTAWA_CFGOUTPUT_H_ */
