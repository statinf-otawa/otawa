/*
 *	ai module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include "ai.h"
#include <otawa/dfa/ai.h>
#include <otawa/ai/TransparentCFGCollectionGraph.h>

using namespace elm;

namespace otawa { namespace ai {

/**
 * @defgroup ai		New Abstract Interpretation Engine
 *
 * This module defines a set of classes to perform abstract interpretation.
 * It is most versatile as the previous module, @ref HalfAbsInt, and should be used instead
 * in OTAWA to perform analyzes.
 *
 * @section ai-principles Principles
 *
 * The support for abstract interpretation is made of different classes that
 * are able to interact together according to the needs of the developer.
 * Basically, there are 3 types of classes:
 * @li drivers -- implement the Iterator model, they implement the policy to manage analysis order (@ref WorkListDriver),
 * @li stores -- store and retrieve efficiently results of abstract interpretation (@ref ArrayStore, @ref EdgeStore),
 * @li graphs -- provide interface with program representation (as graphs) and other classes (@ref CFGGraph).
 *
 * Finally, the domains are given by the user and details the abstract interpretation to perform.
 *
 * @section ai-method Method
 *
 * First, according to the program representation, developer has to choose a graph adapter. Currently, there is only two
 * but more will be proposed in the near future:
 * * @ref CFGGraph -- to analyze a single CFG,
 * * @ref CFGCollectionGraph -- to analyze a set of CFGs representing a task (CFGs are calling each other).
 *
 * Second, the abstract interpretation domain has to be designed. This is class providing the following resources:
 * @li empty constructor (for extend freedom in implementation of stage),
 * @li t -- type of values in the abstract domain,
 * @li init() -- return the initial state (at entry node of the graph),
 * @li bot() -- bottom value,
 * @li join() -- join of two values of the abstract domain,
 * @li update() -- update operation for the state (domain dependent).
 *
 * The domain must match the @ref otawa::ai::DomainConcept.
 *
 * Then the storage and the driver must be chosen and tied together as below. This approach allows to customize
 * the way the computation is performed. In the example below, we just apply the same method whatever the node
 * or the edge.
 *
 * @code
 *  MyDomain dom;
 * 	CFGGraph graph(cfg);
 * 	ArrayStore<MyDomain, CFGraph> store;
 *  WorkListDriver<MyDomain, CFGGraph, ArrayStore<MyDomain, CFGraph> > driver(dom, graph, store);
 *  while(driver) {
 *  	MyDomain::t x;
 *  	dom.copy(x, driver.input());
 *  	update(*driver, x);
 *  	driver.check(x);
 *  }
 * @endcode
 * Where update() is a dedicated function to update the input state according to the domain.
 *
 * @section ai-adapter Using adapters
 * To make things a bit simpler to implement, you can use an adapter. An adapter merges
 * together all what is needed to perform the analysis and provides a simple way to
 * provide the `update()` function. An adapter must match the @ref otawa::ai::Adapter concept
 * and can be used with some analyzes like @ref otawa::ai::SimpleAI.
 *
 * Let your adapter class named `MyAdapter`, the analysis is simply launched with:
 * @code
 * MyAdapter adapter;
 * SimpleAI<MyAdapter> ana(adapter);
 * ana.run();
 * @endcode
 * And then, you can collect the result of the analysis in your store object.
 *
 * The adapter must all required information. For the example of the previous section,
 * the adapter will be:
 * @code
 * class MyAdapter {
 * public:
 *
 * 	void update(Block *b, typename MyDomain::t& d) {
 * 		// perform the update here on d
 * 	}
 *
 *	inline MyDomain& domain(void) { return _domain; }
 *	inline CFGGraph& graph(void) { return _graph; }
 *	inline ArrayStore<MyDomain, CFGGraph>& store(void) { return _store; }
 * private:
 * 	MyDomain _domain;
 * 	CFGGraph _graph;
 * 	ArrayStore<MyDomain, CFGGraph> _store;
 * };
 * @endcode
 *
 */


/**
 * @class WorkListDriver
 * Driver of abstract interpretation with a simple to-do list.
 * @param D		Current domain (must implement @ref otawa::ai::Domain concept).
 * @param G		Graph (must implement @ref otawa::ai::Graph concept).
 * @param S		Storage.
 * @ingroup ai
 */

/**
 * @fn WorkListDriver::WorkListDriver(D& dom, const G& graph, S& store)
 * Initialize the driver.
 * @param dom	Domain.
 * @param graph	Graph to analyze.
 * @param store	Storage to get domain values.
 */
/**
 * @fn bool WorkListDriver::ended(void);
 * Test if the traversal is ended.
 * @return	True if ended, false else.
 */

/**
 * @fn void WorkListDriver::next(void);
 * Go to the next vertex to process.
 */

/**
 * @fn typename G::vertex_t WorkListDriver::item(void);
 * Get the current vertex.
 * @return	Current vertex.
 */

/**
 * @fn void WorkListDriver::change(void);
 * Called when the output state of the current vertex is changed
 * (and successors must be updated).
 */

/**
 * @fn void WorkListDriver::change(typename D::t s);
 * Set the new output state of the current vertex.
 */

/**
 * @fn void WorkListDriver::change(typename G::edge_t edge);
 * Called when the output state of the current vertex for the given edge is changed
 * (and successors must be updated).
 */

/**
 * @fn void WorkListDriver::change(typename G::edge_t edge, typename D::t s);
 * Change the output value of the current vertex for the given edge.
 */

/**
 * @fn void WorkListDriver::changeAll(void);
 * Consider that all has been changed causing a re-computation of all.
 */

/**
 * @fn void WorkListDriver::check(typename G::edge_t edge, typename D::t s);
 * If there is a state change for the given edge.
 */

/**
 * @fn typename D::t WorkListDriver::input(void);
 * Compute the input state.
 */


/**
 * @fn typename D::t WorkListDriver::input(vertex_t vertex);
 * Compute the input state for the given vertex.
 * @param 	vertex	Vertex whose input is required.
 * @return	Input state of vertex.
 */


/**
 * @class CFGGraph
 * BiDiGraph adapter implementation for CFG.
 * @ingroup ai
 */


/**
 * @class ArrayStore
 * Storage of output values as an array.
 * @param D		Domain type.
 * @param G		Graph type.
 * @ingroup ai
 */


/**
 * @class EdgeStore
 * State storage on the edges using properties.
 * @ingroup ai
 */

/**
 * @class CFGCollectionGraph
 *
 * Class representing a graph where an AI (Abstract Interpretation) can be performed.
 * This class is compatible with classes of module @ref ai.
 *
 * It supports the set of CFG making a task: it is viewed with this class a tuple T = (V, E, F, ν, φ) where:
 * @li V is the set of all blocks of all CFG including basic blocks, ν_f or ω_f entry and exit point of function f,
 * @li E ⊆ V × V the set of edges,
 * @li F ⊆ V × V contains the pair of (entry, exit) blocks of the functions in the task,
 * @li ν is the entry of the main function of the task — this means ∃ω ∈ V ∧ (ν, ω) ∈ F,
 * @li φ: V → F ∪ { ⊥ }  associates to a function call the pair of entry, exit blocks of the function or ⊥ if the block is not a call.
 *
 * @ingroup ai
 */


/**
 * @class SimpleAI
 *
 * Class implementing a simple abstract interpretation. It uses the given
 * @ref otawa::ai::AdapaterConcept class and uses for interpreting the graph
 * until reaching a fixpoint.
 *
 * @param A		Actual type of the adapter.
 *
 * @ingroup ai
 */

/**
 * @fn void SimpleAI::run(void);
 * Performed the analysis using the given adapter, that is, basically produces for each vertex
 * of the graph a domain value. The values are computed and propagated along the edges
 * until a fixpoint is found.
 */


/**
 * @class TransparentCFGCollectionGraph
 *
 * This instance of @ref ai graph process the CFG collection of a task as single graph.
 * This means that the @ref SynthBlock, excepted those marked with the exclude identifier passed
 * at construction time, will be invisible for the analysis.
 *
 * Actually, iterating on the successor
 * edges of a call bloc will give the edges between the entry and the first block of the called
 * subprogram. In the same way, iterating on the successor edges of a return block of a function
 * will iterate on the successor of the caller @ref SynthBlock.
 *
 * This works in the same looking to the predecessor: the predecessor of the first blocks of
 * a subprogram will match the predecessor of the call @ref SynthBlock and the predecessors
 * of a block following a call will be the return blocks of a called function.
 *
 * This allow to consider the collection of CFG as one unique graph but an practical outcome
 * is that (a) for successors, the sink vertex is not always the currently processed vertex and
 * (b) for predecessors, the source vertex is not always the currently processed vertex.
 *
 * @ingroup ai
 */

/**
 */
TransparentCFGCollectionGraph::Successor::Successor(Block *b, const TransparentCFGCollectionGraph& g)
: i(b->outs()), _g(g) {
	setup();
}

/**
 */
void TransparentCFGCollectionGraph::Successor::next(void) {
	i++;
	setup();
}

/**
 */
void TransparentCFGCollectionGraph::Successor::setup(void) {

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
			if(i->sink()->toSynth()->callee() == nullptr or _g.isExcluded(i->sink()))
				break;
			else {
				i++;
				todo.push(i);
				i = i->sink()->toSynth()->callee()->entry()->outs();
			}
		}

		// edge to an exit
		else if(i->sink()->isExit()) {
			if(!i->sink()->cfg()->callers())
				break;
			i++;
			todo.push(i);
			CFG::CallerIter c(i->sink()->cfg()->callers());
			i = c->outs();
			c++;
			todo.push(c);
		}

		// unknown
		else
			break;
	}

}

/**
 */
TransparentCFGCollectionGraph::Predecessor::Predecessor(Block *b, const TransparentCFGCollectionGraph& g)
: i(b->ins()), _g(g) {
	setup();
}

/**
 */
void TransparentCFGCollectionGraph::Predecessor::next(void) {
	i++;
	setup();
}

/**
 */
void TransparentCFGCollectionGraph::Predecessor::setup(void) {

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
			if(i->source()->toSynth()->callee() == nullptr or _g.isExcluded(i->source()))
				break;
			else {
				i++;
				todo.push(i);
				i = i->source()->toSynth()->callee()->entry()->ins();
			}
		}

		// edge to an entry
		else if(i->source()->isExit()) {
			if(!i->source()->cfg()->callers())
				break;
			i++;
			todo.push(i);
			CFG::CallerIter c(i->source()->cfg()->callers());
			i = c->ins();
			c++;
			todo.push(c);
		}

		// unknown
		else
			break;
	}
}

} } 	// otawa::ai
