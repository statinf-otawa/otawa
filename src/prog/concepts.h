/*
 *	$Id$
 *	concept collection
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <elm/data/Vector.h>

using namespace elm;

namespace otawa { namespace concept {

/**
 * This concept attempts to provide a representation of a digraph that may only
 * be traversed in the edge direction.
 *
 * @note	A specificity of OTAWA is that each vertex is associated with an
 * 			index in the range of 0 and the count of vertices in the graph.
 * 			This is useful to implement some optimizations.
 */
class DiGraph: public elm::concept::Collection<Vertex> {
public:

	/** Vertex class. */
	class Vertex { };

	/** Opaque type for the edges. */
	class Edge { };

	/**
	 * Get the sink of the given edge.
	 * @param edge	Edge to get target of.
	 * @return		Sink of the given edge.
	 */
	const Vertex& sinkOf(Edge& edge);

	/**
	 * Get the output degree of the vertex.
	 * @param vertex	Vertex to get the out degree.
	 * @return			Out degreee.
	 */
	int outDegree(const Vertex& vertex) const;

	/**
	 * Test if the succ vertex is successor of the ref vertex.
	 * @param succ	Successor vertex.
	 * @param ref	Reference vertex.
	 * @return		True if succ is successor of ref.
	 */
	bool isSuccessorOf(Vertex& succ, const Vertex& ref);

	/** Outing edge iterator on a node. */
	class Successor: public Iterator<Edge> {
	public:

		/**
		 * Build the iterator on the successor edge of the source.
		 * @param source	Source node.
		 */
		Successor(const DiGraph& graph, const Vertex& source);

		/**
		 * Clone the given successor iterator.
		 * @param forward	Iterator to clone.
		 */
		Successor(const OutIterator& iterator);
	};

};


/** A digraph that supports loop identification. */
class DiGraphWithLoop: public DiGraph {

	/**
	 * Test if the vertex is a loop header.
	 * @return		True if it is a loop header, false else.
	 */
	bool isLoopHeader(Vertex& vertex);
};


/** This kind of digraph contain indexed graph. */
class DiGraphWithIndexedVertex: public DiGraph {
public:
	int indexOf(const Vertex& vertex) const;
};


/** Concept of directed graph with predecessor available. */
class BiDiGraph: public DiGraph {

	/** Entering-in edge iterator on a node. */
	class Predecessor: public Iterator<Edge> {
	public:

		/**
		 * Build the iterator on the entering-in edges of the source.
		 * @param source	Source node.
		 */
		Predecessor(const DiGraph& graph, const Vertex& source);

		/**
		 * Clone the given predecessor iterator.
		 * @param forward	Iterator to clone.
		 */
		Predecessor(const InIterator& iterator);
	};

	/**
	 * Get the source of the given edge.
	 * @param edge	Edge to get target of.
	 * @return		Source of the given edge.
	 */
	const Vertex& sourceOf(const Edge& edge);

	/**
	 * Get the output degree of the vertex.
	 * @param vertex	Vertex to get the out degree.
	 * @return			In degree.
	 */
	int inDegree(const Vertex& vertex) const;

	/**
	 * Test if the pred vertex is predecessor of the ref vertex.
	 * @param pred	Predecessor vertex.
	 * @param ref	Reference vertex.
	 * @return		True if pred is predecessor of ref.
	 */
	bool isSuccessorOf(const Vertex& succ, const Vertex& ref);
};


/** Concept of directed graph providing a vertex map. */
class DiGraphWithVertexMap: public DiGraph {
public:
	/** Efficient map for the nodes. */
	template <class T> class VertexMap: public Map<Vertex, T> { };
};


/** Concept of directed graph providing an edge map. */
class DiGraphWithEdgeMap: public DiGraph {
public:
	/** Efficient map for the edges. */
	template <class T> class EdgeMap: public Map<Edge, T> { };
};


/** Directed graph with a unique entry point. */
class DiGraphWithEntry: public DiGraph {
public:
	/** @return the entry vertex. */
	Vertex entry(void);
};


/** Directed graph with a unique entry and exit points. */
class DiGraphWithExit: public BiDiGraph {
public:
	/** @return the exit vertex. */
	Vertex exit(void);
};

/** An iterable block of instruction. **/
class InstBlock {
public:

	/**
	 * Get the address of the first instruction.
	 * @return	First instruction address.
	 */
	Address address(void);

	/**
	 * Get the size of the block.
	 * @return	Size in bytes.
	 */
	size_t size(void);

	/**
	 * Address succeeding the last byte of the block.
	 * @return	address() + size().
	 */
	Address topAddress(void);

	/**
	 * Count the number of instructions.
	 * @return	Number of instructions.
	 */
	int countInsts(void);

	/**
	 * Iterator on the instruction of the block.
	 */
	class InstIter: public Iterator<Inst *> {
	public:

		/** Build an iterator on the given block. */
		InstIter(const InstBlock *block);

		/** Build an iterator cloning the given one. */
		InstIter(const InstIter& iter);
	};
};


/** Concept used to implements @ref AbsIntLite domain.
 *	The soundness and the termination of the analysis requires
 *	a CPO (complete partial order) denoted "<=" on the domain values.
 */
template <class T, class G>
class AbstractDomain {
public:

	/** Type of the values of the domain. */
	typedef T t;

	/** Get the initial value (at graph entry). */
	t initial(void);

	/** Get the smallest value of the domain "_"
	 *  such that forall d in domain, join(_, d) = joind(d, _) = d */
	t bottom(void);

	/**
	 * Perform the join of two values, d = join(d1, d2)
	 * and must satisfy the property: d1 <= d and d2 <= d.
	 * @param d		First value to join and recipient of the result.
	 * @param s		Second value to join.
	 */
	void join(t& d, t s);

	/**
	 *  Test if two values are equal.
	 * 	@param v1	First value.
	 *  @param v2	Secondd value.
	 *	@return		True if both values are equal. */
	bool equals(t v1, t v2);

	/**
	 *  Assign the value s to the value d.
	 * 	@param d	Assigned value.
	 * 	@param s	Value to assign.
	 */
	void set(t& d, t s);

	/**
	 * Only required if you use debug capabilities. It allows to
	 * display value.
	 * @param out	Stream to output to.
	 * @param c		Value to output.
	 */
	void dump(io::Output& out, t c);

	/**
	 * Update the value according to the given graph vertex.
	 * Notice that this function must be monotonic:
	 * forall domain values v1, v2 such that v1 <= v2, update(v1) <= update(v2).
	 * @param vertex	Current vertex.
	 * @param d			Input value and result value.
	 */
	void update(const typename G::Vertex& vertex, t& d);

};

/** Concept used to implements @ref IterativeDFA problems. */
class IterativeDFAProblem {
public:

	/** Type of the set of the problem. */
	typedef struct Set Set;

	/**
	 * Build a new empty set (set constructor).
	 * @eturn	New empty set.
	 */
	Set *empty(void);

	/**
	 * Build the generating set for the given basic block.
	 * @param bb	Basic block to get generating set from.
	 * @return		Generating set.
	 */
	Set *gen(BasicBlock *bb);

	/**
	 * Build the killing set for the given basic block.
	 * @param bb	Basic block to get killing set from.
	 * @return		killing set.
	 */
	Set *kill(BasicBlock *bb);

	/**
	 * Test if two sets are equals.
	 * @param set1	First set to compare.
	 * @param set2	Second set to compare.
	 * @return	True if they are equals, false else.
	 */
	bool equals(Set *set1, Set *set2);

	/**
	 * Set to empty the given set. This operation is mainly called to allow
	 * re-use of working set.
	 * @param set	Set to reset.
	 */
	void reset(Set *set);

	/**
	 * Merge two sets, as after a selection or at the entry of a loop.
	 * @param set1	First set to merge and result of the merge.
	 * @param set2	Second set to merge.
	 */
	void merge(Set *set1, Set *set2);

	/**
	 * Assign a set to another one.
	 * @param dset	Destination set.
	 * @param tset	Source set.
	 */
	void set(Set *dset, Set *tset);

	/**
	 * Perform union of a set in another one (generate action).
	 * @param dset	First source and destination set.
	 * @param tset	Second source set.
	 */
	void add(Set *dset, Set *tset);

	/**
	 * Makes the difference between two sets (kill action).
	 * @param dset	Destination and first operand set.
	 * @param tset	Second operand set.
	 */
	void diff(Set *dset, Set *tset);

	/**
	 * Free the ressources used by the given set.
	 * @param set	Set to remove.
	 */
	void free(Set *set);
};

}	// dfa


namespace ai {

/**
 * Graph concept for ai module.
 * @ingroup ai
 */
class Graph {
public:

	/**
	 * Graph vertex type.
	 */
	typedef void *vertex_t;

	/**
	 * Graph edge type.
	 */
	typedef void *edge_t;

	/**
	 * Get the entry vertex.
	 * @return	Entry vertex.
	 */
	vertex_t entry(void) const;

	/**
	 * Get the exit vertex.
	 * @return	Exit vertex.
	 */
	vertex_t exit(void) const;

	/**
	 * Get the sink vertex of the given edge.
	 * @param e		Edge to look to.
	 * @return		Sink node.
	 */
	vertex_t sinkOf(edge_t e) const;

	/**
	 * Get the source vertex of the given edge.
	 * @param e		Edge to look to.
	 * @return		Source node.
	 */
	vertex_t sourceOf(edge_t e) const;

	/**
	 * Iterator on the entering edges on the given vertex.
	 */
	class Predecessor {
	public:
		Predecessor(const CFGGraph& graph, vertex_t v);
		Predecessor(const Predecessor& p);
	};

	/**
	 * Iterator on the leaving edges of the given vertex.
	 */
	class Successor {
	public:
		Successor(const CFGGraph& graph, vertex_t v);
		Successor(const Successor& p);
	};

	/**
	 * Iterator on the vertices of the graph.
	 */
	class Iterator {
	public:
		Iterator(const CFGGraph& g);
		Iterator(const Iterator& i);
	};

	/**
	 * Get the index associated with a vertex.
	 * @param v		Looked vertex.
	 * @return		Associated vertex.
	 */
	int index(vertex_t v) const;

	/**
	 * Get the count of vertices.
	 * @return		Count of vertices.
	 */
	int count(void) const;
};


/**
 * Domain concept for ai module.
 * @ingroup ai
 */
class Domain {
public:

	/**
	 * Type of domain values.
	 */
	typedef void *t;

	/**
	 * Get the initial domain value.
	 * @return	Initial value.
	 */
	t init(void);

	/**
	 * Get the bottom value.
	 * @return	Bottom value.
	 */
	t bot(void);

	/**
	 * Join both value.
	 * @param v1	First value.
	 * @param v2	Second value.
	 * @return		Joined values.
	 */
	t join(t v1, t v2);

	/**
	 * Test if both values are equal.
	 * @param v1	First value.
	 * @param v2	Second value.
	 * @return		True if v1 = v2, false else.
	 */
	bool equals(t v1, t v2);

	/**
	 * Compute input value.
	 * @param vertex	Current vertex.
	 * @param ins		Iterator on inputs (implement InputIter concept).
	 * @return			Input result.
	 */
	template <class I>
	t input(Graph::vertex_t vertex, I& ins);

	/**
	 * Produce a new state by updating the given in state
	 * according to the current vertex.
	 * @param vertex	Current vertex.
	 * @param in		Input state.
	 * @param outs		Iterator on output (implements OutputIter concept).
	 */
	template <class O>
	void output(Graph::vertex_t vertex, t in, O& outs);

};

/**
 * Iterator on input of a vertex.
 * @ingroup ai
 */
class InputIter {
public:

	/**
	 * Get current edge.
	 * @return	Current edge.
	 */
	Graph::edge_t edge(void);

	/**
	 * Get the value associated with current edge.
	 * @return	Value of the current edge.
	 */
	const Domain::t& get(void);
};

/**
 * Iterator to set output values.
 * @ingroup ai
 */
class OutputIter {
public:

	/**
	 * Get current edge.
	 * @return	Current edge.
	 */
	Graph::edge_t edge(void);

	/**
	 * Set the value associated with the current edge.
	 * @param	Value of the current edge.
	 */
	void set(const Domain::t& value);

	/**
	 * Set the value for all edges.
	 * @param	Value for the whole set of edges.
	 */
	void setAll(const Domain::t& value);
};

}	// ai

namespace sem {

/**
 * In DFA of OTAWA, intermediate state are only stored at basic block level. To find back
 * state inside the basic block, one has to re-interpret instruction of the basic block.
 * This class provides a generic concept to support re-interpretation of the state and
 * should be efficient-enough to support both paradigm of state representation:
 * @li immutable -- state is an immutable data structure allowing re-use for join and update operations,
 * @li mutable -- state is a mutable data structure modified by join and update opperations.
 */
template <class T>
class StateManager {
public:
	typedef T t;

	/**
	 * Get the bottom value.
	 * @return	Bottom value.
	 */
	t bot(void);

	/**
	 * Free the given state.
	 * @param s		State to delete.
	 */
	void free(t s);

	/**
	 * Test for equality.
	 * @param s1	First state.
	 * @param s2	Second state.
	 * @return		True if they are equal, false else.
	 */
	bool equals(t s1, t s2);

	/**
	 * Build a copy of given state.
	 * @param s		State to copy.
	 * @return		Copied state.
	 */
	t copy(t s);

	/**
	 * Join both state s1 and s2 and release no more used s1 or s2.
	 * @param s1	First state.
	 * @param s2	Second state.
	 * @return		Join of states.
	 */
	t joinPath(t s1, t s2);

	/**
	 * Interpret the given semantic instruction on the given state.
	 * After the update, s is no more used (and may be released if needed).
	 * @param s		State to update.
	 * @param i		Semantic instruction to update with.
	 * @return		State result of update.
	 */
	t updatePath(t s, sem::inst i);
};

}	// sem

}	// otawa
