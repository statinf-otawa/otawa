/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * prog/util_Edge.cpp -- Edge class implementation.
 */

#include <elm/assert.h>
#include <otawa/util/Graph.h>
#include <elm/io.h>

using namespace elm;

namespace otawa { namespace graph {

/**
 * @class Edge
 * This class represents a directed edge between two nodes.
 */


/**
 */
Edge::~Edge(void) {
	//cout << "Edge " << this << " deleted !\n";
}


/**
 * @fn Edge::Edge(Node *source, Node *target);
 * Build an edge between two nodes.
 * @warning Both nodes must be contained in the same graph.
 * @param source	Source node.
 * @param target	Target node.
 */


/**
 * @fn Node *Edge::source(void) const;
 * Get the source node.
 * @return	Source node.
 */


/**
 * @fn Node *Edge::target(void) const;
 * Get the target node.
 * @return	Target node.
 */

} } // otawa::graph
