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
#include <otawa/ai/FlowAwareRanking.h>
#include <otawa/ai/RankingAI.h>
#include <otawa/ai/SimpleWorkList.h>
#include <otawa/dfa/ai.h>
#include <otawa/ai/BlockAnalysis.h>

using namespace elm;

namespace otawa { namespace ai {

/**
 * Concept for an abstract interpretation on a CFG.
 * @ingroup ai
 */
class CFGDomain {
public:

	/// Type of abstract values.
	typedef struct abstract_value_t t;

	/**
	 * Called just before the abstract interpretation to initialize the domain.
	 * @param ws	Current workspace.
	 */
	void setup(WorkSpace *ws);

	/**
	 * Called just after the abstract interpretation to clean up the resources
	 * allocated for the domain.
	 * @param ws	Current workspace.
	 */
	void cleanup(WorkSpace *ws);

	/**
	 * Get the initial abstract state (just before the CFG execution).
	 * @return	Initial abstract state.
	 */
	t init(WorkSpace *ws);

	/**
	 * Get the bottom value.
	 * @return	Bottom abstract state.
	 */
	t bot() const;

	/**
	 * Join two abstract states.
	 * @param x1	First abstract state.
	 * @param x2	Second abstract state.
	 * @return		Joined abstract state.
	 */
	t join(const t& x1, const t& x2) const;

	/**
	 * Update the given abstract state according to the given CFG block.
	 * @param v	CFG block to update with.
	 * @param x	Abstract state to update.
	 * @return	Updated abstract state x.
	 */
	t update(Block *v, const t& x) const;

	/**
	 * Test if two abstract states are equal.
	 * @param x1	First abstract state.
	 * @param x2	Second abstract state.
	 * @return		True if both abstract states are equal, false else.
	 */
	bool equals(const t& x1, const t& x2) const;

	/**
	 * Assign an abstract state to another abstract state.
	 * @param x	Assigned abstract state.
	 * @param y	Abstract state to assign.
	 */
	void set(t& x, const t& y) const;

	/**
	 * Print an abstract state.
	 * @param x		Abstract state to display.
	 * @param out	Output to use.
	 */
	void print(const t& x, Output& out) const;
};


/**
 * @defgroup ai		New Abstract Interpretation Engine
 *
 * This module defines a set of classes to perform abstract interpretation.
 * It is most versatile as the previous module, @ref HalfAbsInt, and should be used instead
 * in OTAWA to perform analyzes.
 *
 * @section ai-processor Implementing an Abstract Interpretation
 *
 * @section ai-principles Principles(deprecated)
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
 * @class RankingAI
 * Perform an abstract interpretation of the domain, store and graph provided
 * in the adapter and uses a ranking function to organize vertices to process
 * the speed up the interpretation and convergence to the fix-point.
 *
 * @param A		Type of the adapter (must implement @ref AdapterConcept).
 * @param R		Type of the ranking function (must implement @ref RankingConcept).
 *
 * @ingroup ai
 */

/**
 * @fn RankingAI::RankingAI(A& adapter, R& rank);
 * Build the abstract interpretation manager.
 * @param adapter	Adapter to use.
 * @param rank		Ranking function (default to @ref PropertyRanking).
 */

/**
 * @fn RankingAI::run(void);
 * Launch the abstract interpretation that halts as soon as a fix-point
 * is reached. The resulting states are recorded for each vertex
 * in the store of the adapter.
 */


/**
 * @class PropertyRanking
 * This class may be used with @ref RankingAI to organize the list of vertices to process
 * according to the rank associated with each vertex. In this case, the vertices must inherit
 * from @ref PropList and the rank is obtained from the @ref RANK_OF property.
 *
 * @ingroup ai
 */

/**
 */
int PropertyRanking::rankOf(const PropList& props) {
	return RANK_OF(props);
}


/**
 * Identifier used to rank the vertices of a graph according to
 * a priority of processing (lower is better). It is particularly used
 * by @ref PropertyRanking class and thereof by @ref Ranking AI.
 *
 * @par Features
 *
 * @par Hooks
 * 	* @ref otawa::BasicBlock
 */
p::id<int> RANK_OF("otawa::ai::RANK_OF", -1);


/**
 * This feature ensures that a ranking has been assigned to each of block of the
 * current CFG collection. The ranking value is an integer giving a priority (lesser
 * is better) to process the block in a static analysis.
 *
 * @par Properties
 * 		* @ref RANK_OF
 *
 * @par Implementation
 * 		* @ref FlowAwareRanking
 */
p::feature RANKING_FEATURE("otawa::ai::RANKING_FEATURE", p::make<FlowAwareRanking>());


/**
 * @class CFGRanking
 *
 * This interface provides ranking for CFG, that is, based on BB. The ranking
 * is an integer providing a hint to order calculation in an abstract
 * interpretation. Lower is the ranker, sooner should the corresponding block
 * be analyzed. The rank has no effect on the result of analysis but may
 * significantly speed up the calculation time.
 *
 * Different policies can be designed for ranking. As default, the
 * FlowAwareRanking processor is used.
 *
 * @ingroup ai
 */

///
CFGRanking::~CFGRanking() {
}

/**
 * @fn int CFGRanking::rankOf(Block *v);
 * Get the rank for the given CFG block.
 * @param v	CFG block to get rank for.
 * @return	Rank of v.
 */


/**
 * This feature ensures that a rank has been calculated for the block of the current CFGs.
 * The rank is used to order the calculation of block in an abstract interpretation.
 *
 * @par Interface
 * CFGRanking
 *
 * @par Implementation
 * * FlowAwareRanking
 *
 * @ingroup ai
 */
p::interfaced_feature<CFGRanking> CFG_RANKING_FEATURE("otawa::ai::CFG_RANKING_FEATURE", p::make<FlowAwareRanking>());


/**
 * @class DefaultBlockStore
 * This class is a store for abstract values of an abstract interpretation.
 * It implements a map between CFG blocks and abstract values. The
 * implementation uses an array idendex by the identifier of the blocks.
 * To work, it uses the abstract domain for abstract value operations.
 * Notice that the init() function must be called before any operation.
 *
 * @param D		Type of the abstract domain.
 * @ingroup ai
 */

/**
 * @fn void DefaultBlockStore::init(const CFGCollection& c, const D& x);
 * Initialize the store (must be called before any operation and only once).
 * @param c		CFG collection to store abstract values for.
 * @param x		Default abstract value.
 */

/**
 * @fn const D& DefaultBlockStore::get(Block *v) const;
 * Get the value stored for a block.
 * @param v	Block for which the abstract value is looked.
 * @return	Abstract value associated with the block v.
 */

/**
 * @fn void DefaultBlockStore::set(Block *v, const D& x);
 * Set the abstract value x associated with block v.
 * @param v	Block to store the value for.
 * @param x	Stored abstract value.
 */

/**
 * @class BlockAnalysis;
 * Processor Implementing an abstract interpretation analysis at the level of
 * CFG blocks, that is, the update function is called on the blocks. The
 * customization, that the implementation of abstract interpretation itself,
 * is based on a generic parameter that must implement the ai::CFGDomain
 * concept.
 *
 * @ingroup ai
 */

/**
 * @fn D& BlockAnalysis::domain();
 * Get the abstract domain instance.
 * @return	Abstract domain instance.
 */

/**
 * @fn t BlockAnalysis::out(Block *v) const;
 * Get the output state for CFG block v.
 * @param v		Looked blocked.
 * @return		Abstract output state for v.
 */

/**
 * @fn t BlockAnalysis::in(Block *v) const;
 * Get the input state for CFG block v.
 * @param v		Looked blocked.
 * @return		Abstract input state for v.
 */


/**
 * @class SimpleWorkList
 * This class implements a very light and simple work list based on a list
 * queue and on a bit vector to avoid duplicates in the list queue. It is
 * dedicated to store CFG blocks.
 * @ingroup ai
 */

/**
 * @fn SimpleWorkList::SimpleWorkList(const CFGCollection& coll);
 * Build a simple work list for the given CFG collection.
 * @param coll	CFG collection to work on.
 */

/**
 * @fn bool SimpleWorkList::isEmpty() const;
 * Test if the work list is empty.
 * @return	True if the work list is empty, false else.
 */

/**
 * @fn void SimpleWorkList::put(Block *v);
 * Add the given block in to the work list.
 * @param v		Added block.
 */

/**
 * @fn Block *SimpleWorkList::get();
 * Get the next block from the work list and remove it. It is illegal to call
 * this function in an empty work list.
 * @return	Next available block in the work list.
 */

} } 	// otawa::ai

