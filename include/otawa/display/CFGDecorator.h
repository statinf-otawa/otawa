/*
 *	display::CFGDisplayer class interface
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
#ifndef OTAWA_DISPLAY_CFGDISPLAYER_H_
#define OTAWA_DISPLAY_CFGDISPLAYER_H_

#include "Displayer.h"
#include <otawa/cfg.h>

namespace otawa { namespace display {

class CFGDecorator: public display::GenDecorator<CFG> {
public:

	CFGDecorator(WorkSpace *ws);

	bool display_assembly;
	bool display_source_line;
	bool display_props;
	Color source_color, label_color;

	void decorate(CFG *graph, Text& caption, GraphStyle& style) const override;
	void decorate(CFG *graph, Block *block, Text& content, VertexStyle& style) const override;
	void decorate(CFG *graph, otawa::Edge *edge, Text& label, EdgeStyle& style) const override;

	void setDisplayOptions(bool _da = true, bool _dsl = true, bool _dp = false, Color _sc = Color("darkgreen"), Color _lc = Color("blue"));

protected:
	virtual void displayEndBlock(CFG *graph, Block *block, Text& content, VertexStyle& style) const;
	virtual void displaySynthBlock(CFG *graph, SynthBlock *block, Text& content, VertexStyle& style) const;
	virtual void displayBasicBlock(CFG *graph, BasicBlock *block, Text& content, VertexStyle& style) const;
	virtual void displayHeader(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayBody(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayAssembly(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayProps(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displaySourceLine(Address addr, Text& content) const;
	virtual void displayLabels(Inst *i, Text& content) const;
	virtual void displayInst(Inst *i, Text& content) const;
	virtual void displayInfo(Inst *i, Text& content) const;

	inline WorkSpace *workspace(void) const { return ws; }

protected:
	WorkSpace *ws;

private:
	mutable cstring file;
	mutable int line;
};

} }		// otawa::display

#endif /* INCLUDE_OTAWA_DISPLAY_CFGDISPLAYER_H_ */
