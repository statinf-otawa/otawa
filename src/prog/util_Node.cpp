/*
 *	graph::Node class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#include <elm/io.h>

#include "../../include/otawa/ograph/Graph.h"
using namespace elm;

namespace otawa { namespace ograph {

/**
 * @class Node
 * The node from a directed graph.
 */


/**
 * @fn Node::Node(Graph *graph = 0);
 * Build a new node.
 * @param graph	Container graph.
 */


/**
 * Unlink the node, that is, remove its entering or leaving edges.
 */
void Node::unlink(void) {
	while(ins)
		_graph->remove(ins);
	while(outs)
		_graph->remove(outs);
}


/**
 */
Node::~Node(void) {
	//cout << "Node " << this << " deleted !\n";
	if(_graph)
		_graph->remove(this);
}


/**
 * @fn Graph *Node::graph(void) const;	
 * Get the container graph if any.
 * @return	Container graph or null.
 */


/**
 * @fn int Node::index(void) const;
 * Get the index of the node in the graph.
 * @return	Node index or -1.
 */


/**
 * @fn bool Node::hasSucc(void) const;
 * Test if the node has successors.
 * @return	True if it has successors, false else.
 */


/**
 * @fn bool Node::hasPred(void) const;
 * Test if the node has predecessors.
 * @return	True if it has predecessors, false else.
 */


/**
 * @fn int Node::countSucc(void) const;	
 * Count the successors of the current node.
 * @return	Count of successors.
 */

/**
 * @fn int Node::countPred(void) const;	
 * Count the predecessors of the current node.
 * @return	Count of predecessors.
 */


/**
 * @fn bool Node::isPredOf(const Node *node);
 * Test if the current node is a predecessor of the given one.
 * @param node	Node to test.
 * @return		True if the current is a predecessor, false else.
 */


/**
 * @fn bool Node::isSuccOf(const Node *node);
 * Test if the current node is a successor of the given one.
 * @param node	Node to test.
 * @return		True if the current is a successor, false else.
 */


/**
 * @class Node::Successor
 * This class allows to iterates on the successors of a node.
 */


/**
 * @fn Node::Successor::Successor(const Node *node);
 * Build an iterator on the successor of the given node.
 * @param node	Node whose successors will be iterated.
 */
 
 
/**
 * @fn Edge *Node::Successor::edge(void) const;
 * Get the edge leading to the current successor.
 * @return	Current edge.
 */


/**
 * @class Node::Predecessor
 * This class allows to iterates on the predecessors of a node.
 */


/**
 * @fn Node::Predecessor::Predecessor(const Node *node);
 * Build an iterator on the predecessors of the given node.
 * @param node	Node whose predecessors will be iterated.
 */
 
 
/**
 * @fn Edge *Node::Predecessor::edge(void) const;
 * Get the edge leading to the current predecessor.
 * @return	Current edge.
 */

} } // otawa::graph
