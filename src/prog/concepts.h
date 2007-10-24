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
class DiGraph: public Collection {
public:

	/** The type of the nodes. */
	class Vertex {
		
		/**
		 * Get the index of the vertex in the graph vertices list.
		 * @return	Vertex index.
		 */
		int index(void);
	};

	/** The edge kind. */
	class Edge {
	public:
		
		/**
		 * Get the source of the edge.
		 * @return	Source vertex.
		 */
		Vertex *source(void) const;
		
		/**
		 * Get the sink node of the edge.
		 * @return	Sink vertex.
		 */
		Vertex *sink(void) const;
	};
	
	/** Forward iterator on a node. */
	class Forward: public Iterator<Edge *> {
	public:
		
		/**
		 * Build the iterator on the outing edge of the source.
		 * @param source	Source node.
		 */
		Forward(Vertex *source);
		
		/**
		 * Clone the given forward iterator.
		 * @param forward	Iterator to clone.
		 */
		Forward(const Forward& forward);
	};
	
	/**
	 * Get the entry of the digraph.
	 * @return	Entry vertex.
	 */
	Vertex *entry(void);
	
	/**
	 * Get the count of vertices in the graph.
	 * @return	Vertices count.
	 */
	int count(void);
};


/**
 * A graph that may be traversed in both directions.
 */
class BiDiGraph: public DiGraph {
public:
	
	/** Iterator back from a node. */
	class Backward: public Iterator<Edge *> {
	public:
		
		/**
		 * Constructor from the sink node.
		 * @param sink	Sink node.
		 */
		Backward(Vertex *sink);
		
		/**
		 * Constructor by cloning.
		 * @param backward	Iterator to clone.
		 */
		Backward(const Backward& backward);
	};
	
	/**
	 * In bidirectionnal digraph, the exit vertex.
	 * @return	Exit vertex.
	 */
	Vertex *exit(void);
};


/**
 * A direction provides a way to traverse a graph. There is usually two
 * implementation: @ref otawa::graph::ForwardDirection and
 * @ref otawa::graph::BackwardDirection.
 * @param G	Type of graph.
 */
template <class G>
class Direction {
public:
	
	/**
	 * Get the start vertex to traverse the graph.
	 * @return	Start vertex.
	 */
	G::Vertex *start(void);
	
	/** Iterator the next vertices. */
	class Next: public Iterator<G::Vertex *> {
	public:
		
		/**
		 * Build the iterator on thegiven vertex.
		 * @param vertex	Vertex to look for next vertics.
		 */
		Next(G::Vertex *vertex);
		
		/**
		 * Build the iterator by cloning.
		 * @param next	Iterator to clone.
		 */
		Next(const Next& next);
	};
};


/**
 * Concept of class that may process graphs.
 * @param G		Graph type (@ref DiGraph concept).
 */
template <class G>
class VertexProcessor {
public:
	
	/**
	 * Called to process a vertex.
	 * @param graph		Container graph.
	 * @param vertex	Current vertex.
	 */
	void process(G *graph, G::Vertex *vertex);
};


/**
 * All algorithm implementing a graph traversal must implement this concept.
 * @param P		Vertex processor type (@ref VertexProcessor concept).
 * @param G		Type of graph (@ref DiGraph concept).
 * @param D		Direction of traversal (@ref Direction concept).
 * 
 * The algorithm is used as in the example below:
 * @code
 * MyVertexProcessor processor;
 * MyGraph *graph;
 * MyAlgo<MyVertexProcessor, MyGraph> algo(processor, graph);
 * @endcode
 */
template <class P, class G, template <class T> class D = ForwardDirection<T> >
class TraversalAlgo {
public:
	
	/**
	 * Launch the traversal of the graph.
	 * @param processor	Vertex processor to use.
	 * @param graph		Graph to traverse.
	 */
	TraversalAlgo(P& processor, G *graph);
};

} } // otawa::concept
