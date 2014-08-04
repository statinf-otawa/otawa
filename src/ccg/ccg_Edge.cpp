/*
 *	CCEdge class implementation
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
#include <include/otawa/cfg.h>
#include <otawa/ccg/Edge.h>
#include <otawa/ccg/Node.h>

namespace otawa { namespace ccg {

/**
 * @class Edge
 * This class represents edges in the CCG representation.
 * They allow hooking annotations.
 */
 
/**
 * Build a new CCG edge.
 */
Edge::Edge(Node *source, Node *target) :
	GenGraph<Node, Edge>::GenEdge(source,target)
{
}

/**
 * Delete an edge.
 */
Edge::~Edge(void) {
}

} }	// otawa::ccg
