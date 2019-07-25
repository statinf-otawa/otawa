/*
 *	MultipleDotDisplayer interface
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
#ifndef OTAWA_DISPLAY_MULTIPLE_DOT_DISPLAYER_H
#define OTAWA_DISPLAY_MULTIPLE_DOT_DISPLAYER_H

#include "../../include/otawa/display/CFGDecorator.h"

using namespace otawa;

namespace mkff {

class MultipleDotDecorator: public display::CFGDecorator {
public:
	inline MultipleDotDecorator(WorkSpace *ws): display::CFGDecorator(ws) { }
protected:
	virtual void displaySynthBlock(CFG *g, SynthBlock *b, display::Text& content, display::VertexStyle& style) const;
	virtual void displayEndBlock(CFG *graph, Block *block, display::Text& content, display::VertexStyle& style) const;
	virtual void displayBasicBlock(CFG *graph, BasicBlock *block, display::Text& content, display::VertexStyle& style) const;
	virtual void decorate(CFG *graph, otawa::Edge *edge, display::Text& label, display::EdgeStyle& style) const;

	virtual void displayProps(CFG *g, BasicBlock *b, display::Text& content) const;
}; // class MultipleDotDecorator: public display::CFGDecorator {

} // namespace mkff

#endif	// OTAWA_DISPLAY_MULTIPLE_DOT_DISPLAYER_H
