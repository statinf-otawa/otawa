/*
 *	features for CCG plugin
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_CCG_FEATURES_H_
#define OTAWA_CCG_FEATURES_H_

#include <otawa/util/GenGraph.h>
#include <otawa/proc/Feature.h>

namespace otawa {

namespace ilp { class Var; }

namespace ccg {

class Node;
class Edge;
class Collection;

// Graph class
class Graph: public GenGraph<Node, Edge> {
public:

	// Properties
	static Identifier<Node *> NODE;
	static Identifier<Collection *> GRAPHS;
};

// CCG feature
extern p::feature FEATURE;

// constraint feature
extern Identifier<ilp::Var *> MISS_VAR;
extern p::feature CONSTRAINT_FEATURE;

} }		// otawa::ccg

#endif /* OTAWA_CCG_FEATURES_H_ */
