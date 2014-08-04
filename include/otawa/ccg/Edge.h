/*
 *	CCGEdge class interface
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
#ifndef OTAWA_CCG_CCGEDGE_H
#define OTAWA_CCG_CCGEDGE_H

#include <otawa/ilp/Var.h>
#include <otawa/util/GenGraph.h>

namespace otawa { namespace ccg {

// Classes
class Node;
class Var;

// Edge class
class Edge: public GenGraph<Node, Edge>::GenEdge, public PropList {
public:
	Edge(Node *source, Node *target);
	~Edge(void);
};


} } // otawa::ccg

#endif	// OTAWA_CCG_CCG_CCGEDGE_H
