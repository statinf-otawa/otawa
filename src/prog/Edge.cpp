/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/Edge.cpp -- implementation of Edge class.
 */

#include <otawa/cfg.h>

namespace otawa {

/**
 * @class Edge
 * This class represents edges in the CFG representation.
 * They allow hooking annotations.
 */
 
 
/**
 * Build a new edge.
 */
Edge::Edge(BasicBlock *source, BasicBlock *target, edge_kind_t kind)
: src(source), tgt(target), knd(kind) {
	assert(source);
	assert(target);
	assert(kind != EDGE_Null);
	src->addOutEdge(this);
	if(knd != CALL)
		tgt->addInEdge(this);
}


/**
 * Delete an edge.
 */
Edge::~Edge(void) {
	src->removeOutEdge(this);
	if(knd != CALL)
		tgt->removeInEdge(this);
}


/**
 * Transform this edge as a call edge.
 */
void Edge::toCall(void) {
	if(knd != CALL) {
		tgt->removeInEdge(this);
		knd = CALL;
	}
}

} // otawa
