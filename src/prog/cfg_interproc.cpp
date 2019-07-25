/*
 *	interproc classes definition
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

#include <otawa/cfg/interproc.h>

/** Development Note
 * 	Letting the ToDo / ToDoList class visible in the header is clumsy.
 * 	We should a find a way to remove it from header as it is mainly
 * 	a linked list of ToDo objects.
 */

namespace otawa { namespace ip {

/**
 * @class Successor
 *
 * The Successor class is an iterator on nodes / edge following a particular block
 * and presenting the task CFG collection as a unique graph (hiding the limits between CFGs).
 * Depending on  the node kind
 * the successors may correspond to real successor in CFG or, around a call block,
 * to the corresponding blocks in the called graph:
 *	* successors of call blocks (if callee defined) are the first block of the callee CFG (following entry),
 *	* successors of exit blocks of a called CFG are the successors of its caller blocks,
 *	* successors of other blocks are usual CFG successors.
 *
 * @ingroup cfg
 */

/**
 */
Successor::Successor(Block *b): i(b->outs()) {
	setup();
}

/**
 */
void Successor::next(void) {
	i++;
	setup();
}

/**
 */
void Successor::setup(void) {

	// primary iterator ended
	while(!i) {

		// no more todo
		if(todo.type() == ToDo::NONE)
			return;

		// another call to process
		else if(todo.type() == ToDo::ITER)  {
			i = todo.asEdge();
			todo.pop();
		}

		// another sub-iteration to process
		else{
			CFG::CallerIter& c = todo.asCall();
			i = c->outs();
			c++;
			if(!c)
				todo.pop();
		}
	}

	// explore to find a basic block
	while(!i->sink()->isBasic()) {

		// edge to a synthetic block
		if(i->sink()->isSynth()) {
			if(i->sink()->toSynth()->callee() == nullptr)
				break;
			else {
				CFG *cfg = i->sink()->toSynth()->callee();
				i++;
				if(i())
					todo.push(i);
				i = cfg->entry()->outs();
			}
		}

		// edge to an exit
		else if(i->sink()->isExit()) {
			if(!i->sink()->cfg()->callers())
				break;
			CFG::CallerIter c(i->sink()->cfg()->callers());
			i++;
			if(i())
				todo.push(i);
			i = c->outs();
			c++;
			if(c())
				todo.push(c);
		}

		// unknown
		else
			break;
	}

}


/**
 * @class Predecessor
 *
 * The Predecessor class is an iterator on nodes / edge preceding a particular block
 * and presenting the task CFG collection as a unique graph (hiding the limits between CFGs).
 * Depending on  the node kind
 * the predecessors may correspond to real predecessor in CFG or, around a call block,
 * to the corresponding blocks in the called graph:
 *	* predecessors of call blocks (if callee defined) are the last blocks of the callee CFG (preceding exit block),
 *	* predecessors of entry blocks of a called CFG are the predecessors of its caller blocks,
 *	* predecessors of other blocks are usual CFG predecessors.
 *
 * @ingroup cfg
 */

/**
 */
Predecessor::Predecessor(Block *b): i(b->ins()) {
	ASSERT(b);
	setup();
}


/**
 */
void Predecessor::next(void) {
	i++;
	setup();
}

/**
 */
void Predecessor::setup(void) {

	// primary iterator ended
	while(!i) {

		// no more todo
		if(todo.type() == ToDo::NONE)
			return;

		// another call to process
		else if(todo.type() == ToDo::ITER)  {
			i = todo.asEdge();
			todo.pop();
		}

		// another sub-iteration to process
		else{
			CFG::CallerIter& c = todo.asCall();
			i = c->ins();
			c++;
			if(!c)
				todo.pop();
		}
	}

	// explore to find a basic block
	while(!i->source()->isBasic()) {

		// edge from a synthetic block
		if(i->source()->isSynth()) {
			if(i->source()->toSynth()->callee() == nullptr)
				break;
			else {
				CFG *cfg = i->source()->toSynth()->callee();
				i++;
				if(i())
					todo.push(i);
				i = cfg->exit()->ins();
			}
		}

		// edge from an entry
		else if(i->source()->isEntry()) {
			if(!i->source()->cfg()->callers())
				break;
			CFG::CallerIter c(i->source()->cfg()->callers());
			i++;
			if(i())
				todo.push(i);
			i = c->ins();
			c++;
			if(c())
				todo.push(c);
		}

		// unknown
		else
			break;
	}
}

/**
 * For internal use only.
 */
ToDo ToDo::null;

} }	// otawa::ip
