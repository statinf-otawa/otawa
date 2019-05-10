/*
 *	PCG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_PCG_PCG_H
#define OTAWA_PCG_PCG_H

#include <elm/PreIterator.h>
#include <otawa/cfg/CFG.h>
#include "../graph/DiGraph.h"
#include "PCGBlock.h"

namespace otawa {
	
using namespace elm::io;

class PCGBlock;

class PCGEdge: public PropList, public graph::GenEdge<PCGBlock, PCGEdge>::GenEdge {
public:
	PCGEdge(SynthBlock *block);
	inline SynthBlock *call(void) const { return _block; }
	inline PCGBlock *caller(void) const { return source(); }
	inline PCGBlock *callee(void) const { return sink(); }
private:
	SynthBlock *_block;
};

class PCGBlock: public PropList, public graph::GenVertex<PCGBlock, PCGEdge> {
public:
	PCGBlock(CFG *cfg);
	~PCGBlock();

	bool isUndef() const;
	address_t address() const;
	string name() const;
	CFG *cfg() const;

	inline address_t getAddress() const { return address(); }
	inline string getName() const { return name(); }

private:
	CFG *_cfg;
};

class PCG: public PropList, public graph::GenDiGraph<PCGBlock, PCGEdge> {
	friend class PCGBuilder;
public:
	typedef graph::GenDiGraph<PCGBlock, PCGEdge>::VertexIter Iter;
	inline PCGBlock *entry(void) const { return _entry; }
	inline Iter blocks(void) const { return vertices(); }
private:
	PCGBlock *_entry;
};

io::Output& operator<<(io::Output& out, PCGBlock *b);
io::Output& operator<<(io::Output& out, PCGEdge *b);

}	// otawa

#endif
