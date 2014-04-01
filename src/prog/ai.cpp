/*
 *	ai module interface
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

#include <otawa/dfa/ai.h>

namespace otawa { namespace ai {

/**
 * @defgroup ai		New Abstract Interpretation Engine
 *
 * This module defines a set of classes to perform abstract interpretation.
 * It is most versatile as the previous module, HalfAbsInt, and should be used
 * to perform now in OTAWA to perform analyzes.
 *
 * @section ai-principles Principles
 *
 * The support for abstract interpretation is made of different classes that
 * are able to interact together according the needs of the developer.
 * Basically, there are three types of classes:
 * @li drivers -- implement the Iterator model, they implement the policy of calculation order (@ref WorkListDriver),
 * @li stores -- store and retrieve efficiently results of abstract interpretation (@ref ArrayStore, @ref EdgeStore),
 * @li adapters -- provide interface between program representation (as graphs) and other classes (@ref CFGGraph).
 *
 * Finally, the domains are given by the user and details the abstract interpretation to perform.
 *
 * @section ai-method Method
 *
 * First, according to the program representation, developer has to choose a graph adapter. There is only one now
 * (@ref CFGGraph) for a lonely CFG but more will be proposed in the near future.
 *
 * Second, the abstract interpretation domain has to be designed. It is class providing the following resources:
 * @li empty constructor (for extend freedom in implementation of stage),
 * @li t -- type of values in the abstract domain,
 * @li init() -- return the initial state (at entry node of the graph),
 * @li join() -- join of two values of the abstract domain.
 *
 * Update functions are not included in this list because they depend only on the way driver is used.
 *
 * Then the storage and the driver must be chosen and tied together as below. This approach allows to customize
 * the way the computation is performed. In the example below, we just apply the same method whatever the node
 * or the edge.
 *
 * @code
 *  MyDomain dom;
 * 	CFGGraph graph(cfg);
 * 	ArrayStore<MyDomain, CFGraph> store;
 *  WorkListDriver<MyDomain, CFGGraph, ArrayStore<MyDomain, CFGraph> > driver(dom, graph store);
 *  while(driver)
 *  	driver.check(dom.update(*driver, driver.input()));
 * @endcode
 */


/**
 * @class WorkListDriver
 * Driver of abstract interpretation with a simple to-do list.
 * @param D		Current domain (must implement otawa::ai::Domain concept).
 * @param G		Graph (must implement otawa::ai::Graph concept).
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
 * State storage on the edges.
 * @ingroup ai
 */

} } 	// otawa::ai
