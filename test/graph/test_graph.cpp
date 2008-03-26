/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 * 
 * tests/graph/test_graph.cpp -- Test program for Graph class.
 */

#include <assert.h>
#include <elm/io.h>
#include <otawa/util/Graph.h>

using namespace elm;
using namespace otawa::graph;

class MyNode: public Node {
public:
	int num;
	MyNode(Graph *graph, int _num): Node(graph), num(_num) { };
};

int main(void) {
	
	// Create all
	Graph graph;
	MyNode *nodes[3];
	nodes[0] = new MyNode(&graph, 0),
	nodes[1] = new MyNode(&graph, 1),
	nodes[2] = new MyNode(&graph, 2);
	new Edge(nodes[0], nodes[1]);
	new Edge(nodes[0], nodes[0]);
	new Edge(nodes[0], nodes[2]);
	new Edge(nodes[2], nodes[1]);
	new Edge(nodes[2], nodes[1]);
	
	// Display nodes
	for(int i = 0; i < 3; i++) {
		cout << "Node " << i << "\nIN [" << nodes[i]->countPred() << "] ";
		for(Node::Predecessor pred(nodes[i]); pred; pred++)
			cout << ((MyNode *)*pred)->num << ' ';
		cout << "\nOUT [" << nodes[i]->countSucc() << "] ";
		for(Node::Successor succ(nodes[i]); succ; succ++)
			cout << ((MyNode *)*succ)->num << ' ';
		cout << '\n';
	}
	
	// Display in preorder
	cout << "PREORDER = ";
	for(Graph::PreorderIterator node(&graph, nodes[0]); node; node++)
		cout << ((MyNode *)*node)->num << " ";
	cout << "\n";
	
	// Destroy all
	/*delete nodes[1];
	delete nodes[2];
	delete nodes[0];*/
}
