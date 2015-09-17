/*
 *	$Id$
 *	PFG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_CFG_PFG_H_
#define OTAWA_CFG_PFG_H_

#include <elm/genstruct/HashTable.h>
#include <otawa/prop/Identifier.h>
#include <otawa/prog/Inst.h>
#include <otawa/graph/GenGraph.h>

namespace otawa { namespace pfg {

using namespace elm;
class BB;
class Edge;
typedef otawa::GenGraph<BB, Edge> graph_t;
class PFG;

// BasicBlock class
class BB: public graph_t::GenNode {
public:

	inline BB(void): _first(0), _size(0) { }
	inline BB(Inst *first, t::uint32 size): _first(first), _size(size) { }
	inline Address address(void) const { return _first->address(); }
	inline Address topAddress(void) const { return address() + _size; }
	inline t::uint32 size(void) const { return _size; }
	inline Inst *first(void) const { return _first; }
	inline int count(void) const;
	inline PFG& pfg(void) const;
	bool isControl(void) const;
	bool isBranch(void) const;
	bool isConditional(void) const;
	bool isBranched(void) const;
	bool isCall(void) const;
	bool isReturn(void) const;
	bool isMultiTarget(void) const;
	inline void setSize(size_t size) { _size = size; }

	// InstIter class
	class InstIter: public PreIterator<InstIter, Inst *> {
	public:
		inline InstIter(const BB *bb): inst(bb->first()), top(bb->topAddress()) { }
		inline InstIter(const InstIter& iter): inst(iter.inst), top(iter.top) { }
		inline InstIter& operator=(const InstIter& iter)
			{ inst = iter.inst; top = iter.top; return *this; }
		
		inline bool ended(void) const { return !inst || inst->address() >= top; }
		inline Inst *item(void) const { return inst; }
		inline void next(void) { inst = inst->nextInst(); }
		  
	private:
		Inst *inst;
		Address top;
	};

private:
	Inst *_first;
	t::uint32 _size;
};

// Edge class
class Edge: public graph_t::GenEdge {
public:
	typedef enum kind_t {
		NONE = 0,
		SEQ = 1,
		BRANCH = 2,
		COND_BRANCH = 3,
		CALL = 4,
		COND_CALL = 5,
		RETURN = 6,
		COND_RETURN = 7
	} kind_t;
	
	inline Edge(BB *source, BB *sink, kind_t kind)
		: graph_t::GenEdge(source, sink), _kind(kind) { }
	inline kind_t kind(void) const { return _kind; }

private:
	kind_t _kind;
};

// PFG class
class PFG: public graph_t {
public:
	
	BB ret;
	BB unknown;

	PFG(void);
	void add(BB *bb);
	BB *get(Address addr);

private:
	genstruct::HashTable<Address, BB *> bbs;
};

} } // otawa::pfg

#endif /* OTAWA_CFG_PFG_H_ */

