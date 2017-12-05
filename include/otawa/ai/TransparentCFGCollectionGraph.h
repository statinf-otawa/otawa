/*
 *	TransparentCFGCollectionGraph class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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
#ifndef OTAWA_AI_TRANSPARENTCFGCOLLECTIONGRAPH_H_
#define OTAWA_AI_TRANSPARENTCFGCOLLECTIONGRAPH_H_

#include <elm/data/List.h>
#include <otawa/cfg/features.h>
#include <otawa/properties.h>

namespace otawa { namespace ai {

class TransparentCFGCollectionGraph {

	class ToDo {
	public:
		typedef enum {
			NONE = 0,
			ITER,
			CALL
		} type_t;
		inline ToDo(type_t t = NONE, ToDo *n = nullptr): type(t), next(n) { }
		type_t type;
		ToDo *next;
		static ToDo null;
	};

	class EdgeToDo: public ToDo {
	public:
		inline EdgeToDo(Block::EdgeIter i, ToDo *n): ToDo(ITER, n), iter(i) { }
		Block::EdgeIter iter;
	};

	class CallToDo: public ToDo {
	public:
		inline CallToDo(CFG::CallerIter i, ToDo *n): ToDo(CALL, n), iter(i) { }
		CFG::CallerIter iter;
	};

	class ToDoList {
	public:
		inline ~ToDoList(void) { while(_hd != &ToDo::null) pop(); }
		inline ToDo::type_t type(void) const { return _hd->type; }
		inline void pop(void) { _hd = _hd->next; }
		inline void push(Block::EdgeIter i) { ASSERT(i); _hd = new EdgeToDo(i, _hd); }
		inline void push(CFG::CallerIter i) { ASSERT(i); _hd = new CallToDo(i, _hd); }
		inline Block::EdgeIter& asEdge(void) const { return static_cast<EdgeToDo *>(_hd)->iter; }
		inline CFG::CallerIter& asCall(void) const { return static_cast<CallToDo *>(_hd)->iter; }
	private:
		ToDo *_hd = &ToDo::null;
	};

public:
	TransparentCFGCollectionGraph(const CFGCollection& coll, AbstractIdentifier *exclude = nullptr)
		: _coll(coll), _exclude(exclude) { }

	typedef Block *vertex_t;
	typedef Edge *edge_t;
	typedef CFG::CallerIter Callers;

	inline AbstractIdentifier *exclude(void) const { return _exclude; }
	inline bool isExcluded(Block *b) const { return _exclude != nullptr && b->hasProp(*_exclude); }

	inline vertex_t entry(void) const { return _coll.entry()->entry(); }
	inline vertex_t exit(void) const { return _coll.entry()->exit(); }
	inline vertex_t sinkOf(edge_t e) const { if(!e->sink()->isSynth()) return e->sink(); else return e->sink()->toSynth()->callee()->entry(); }
	inline vertex_t sourceOf(edge_t e) const { if(!e->source()->isSynth()) return e->source(); else return e->source()->toSynth()->callee()->exit(); }

	inline bool isEntry(vertex_t v) const { return v->isEntry(); }
	inline bool isExit(vertex_t v) const { return v->isExit(); }
	inline bool isCall(vertex_t v) const { return v->isSynth() && v->toSynth()->callee(); }
	inline vertex_t entryOf(vertex_t v) const { return v->toSynth()->callee()->entry(); }
	inline vertex_t exitOf(vertex_t v) const { return v->toSynth()->callee()->exit(); }
	inline Callers callers(vertex_t v) const { return v->cfg()->callers(); }

	class Iterator: public CFGCollection::BlockIter {
	public:
		inline Iterator(const TransparentCFGCollectionGraph& g): CFGCollection::BlockIter(&g._coll) { }
	};

	class Successor: public PreIterator<Successor, Edge *> {
	public:
		Successor(Block *b, const TransparentCFGCollectionGraph& g);
		inline bool ended(void) const { return i.ended(); }
		inline Edge *item(void) const { return *i; }
		void next(void);
	private:
		void setup(void);
		Block::EdgeIter i;
		ToDoList todo;
		const TransparentCFGCollectionGraph& _g;
	};
	inline Successor succs(vertex_t v) const { return Successor(v, *this); }

	class Predecessor: public PreIterator<Predecessor, Edge *> {
	public:
		Predecessor(Block *b, const TransparentCFGCollectionGraph& g);
		inline bool ended(void) const { return i.ended(); }
		inline Edge *item(void) const { return *i; }
		void next(void);
	private:
		void setup(void);
		Block::EdgeIter i;
		ToDoList todo;
		const TransparentCFGCollectionGraph& _g;
	};
	inline Predecessor preds(vertex_t v) const { return Predecessor(v, *this); }

	// Indexed concept
	inline int index(Block *v) const { return v->id(); }
	inline int count(void) const { return _coll.countBlocks(); }

private:
	const CFGCollection& _coll;
	AbstractIdentifier *_exclude;
};

} }		// otawa::ai

#endif /* OTAWA_AI_TRANSPARENTCFGCOLLECTIONGRAPH_H_ */

