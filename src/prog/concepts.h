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
namespace otawa { namespace concept {

/**
 * This concept attempts to provide a representation of a digraph that may only
 * be traversed in the edge direction.
 * 
 * @note	A specificity of OTAWA is that each vertex is associated with an
 * 			index in the range of 0 and the count of vertices in the graph.
 * 			This is useful to implement some optimizations. 
 */
class DiGraph: public Collection<Vertex> {
public:

	/** Vertex class. */
	class Vertex {
	};
	
	/** Opaque type for the edges. */
	class Edge {
	public:
		/** Get the source vertex of the edge. */
		Vertex source(void) const;
		
		/** Get the sink vertex of the edge. */
		Vertex sink(void) const;
	};

	/** Forward iterator on a node. */
	class Successor: public Iterator<Edge> {
	public:
		
		/**
		 * Build the iterator on the outing edge of the source.
		 * @param source	Source node.
		 */
		Successor(const DiGraph& graph, const Vertex& source);
		
		/**
		 * Clone the given forward iterator.
		 * @param forward	Iterator to clone.
		 */
		Successor(const Forward& forward);
	};
	
	/**
	 * Get the output degree of the vertex.
	 * @param vertex	Vertex to get the out degree.
	 * @return			Out degreee.
	 */
	int outDegree(const Vertex& vertex) const;
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
class DiGraphWithEntryAndExit: public DiGraphWithEntry {
public:
	/** @return the exit vertex. */
	Vertex exit(void);
};

} // concept

namespace dfa {

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

}	// otawa
