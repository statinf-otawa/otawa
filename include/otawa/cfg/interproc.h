/*
 *	interproc classes declaration
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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
#ifndef OTAWA_CFG_INTERPROC_H_
#define OTAWA_CFG_INTERPROC_H_

#include "features.h"

namespace otawa { namespace ip {

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

	class Successor: public PreIterator<Successor, Edge *> {
	public:
		Successor(Block *b);
		inline bool ended(void) const { return i.ended(); }
		inline Edge *item(void) const { return *i; }
		void next(void);
	private:
		void setup(void);
		Block::EdgeIter i;
		ToDoList todo;
	};

	class Predecessor: public PreIterator<Predecessor, Edge *> {
	public:
		Predecessor(Block *b);
		inline bool ended(void) const { return i.ended(); }
		inline Edge *item(void) const { return *i; }
		void next(void);
	private:
		void setup(void);
		Block::EdgeIter i;
		ToDoList todo;
	};

} }		// otawa:ip

#endif /* OTAWA_CFG_INTERPROC_H_ */
