/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/Edge.cpp -- implementation of Edge class.
 */

#include <otawa/cfg.h>

namespace otawa {


/**
 * @enum  Edge::kind_t
 * Kind of the edge.
 */

/**
 * @var Edge::kind_t Edge::NONE
 * No meaning. For syntactic use only.
 */


/**
 * @var Edge::kind_t Edge::TAKEN
 * Kind of an edge matching a branch instruction.
 */


/**
 * @var Edge::kind_t Edge::NOT_TAKEN
 * Kind of an edge matching the natural sequential control flow between a
 * basic block and its successor.
 */


/**
 * @var Edge::kind_t Edge::CALL
 * Kind of an edge matching a sub-program call. Edges of this kind provides
 * a calledCFG() value.
 */


/**
 * @var Edge::kind_t Edge::VIRTUAL
 * Kind of an edge linking the virtual entry and exit basic blocks with other
 * ones of the CFG. Does not match any instruction in the sub-program.
 */


/**
 * @var Edge::kind_t Edge::VIRTUAL_CALL
 * Edges of this kind are only found in a virtual CFG. They replace CALL edges
 * whose CFG has been embedded.
 */


/**
 * @var Edge::kind_t Edge::VIRTUAL_RETURN
 * Edges of this kind are only found in a virtual CFG. When a subprogram call
 * is embedded in the main CFG, they link the last basic block of the called
 * subprogram with the basic block successors of the calling basic block.
 */


/**
 * @var Edge::kind_t Edge::EXN_CALL
 * Edges of this kind are usually added after an analysis of the CFG
 * to denote raise of exception (system call, error, etc). The body
 * of the exception handler is linked with an edge of type EXN_CALL
 * while the return is linked with an edge of type EXN_RETURN.
 */


/**
 * @var Edge::kind_t Edge::EXN_CALL
 * Edges of this kind are usually added after an analysis of the CFG
 * to denote raise of exception (system call, error, etc). The body
 * of the exception handler is linked with an edge of type EXN_CALL
 * while the return is linked with an edge of type EXN_RETURN.
 */


/**
 * @class Edge
 * This class represents edges in the CFG representation.
 * They allow hooking annotations.
 */
 
 
/**
 * Build a new edge.
 */
Edge::Edge(BasicBlock *source, BasicBlock *target, edge_kind_t kind)
: knd(kind), src(source), tgt(target) {
	ASSERTP(source, "null source edge");
	ASSERTP(target, "null target edge");
	ASSERTP(kind != EDGE_Null, "null kind edge");
	ASSERTP(kind == CALL || source->cfg() == target->cfg(), "nodes in different CFG");
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
	tgt->removeInEdge(this);
	knd = CALL;
	src->flags |= BasicBlock::FLAG_Call;
}


/**
 * Get the name of a kind.
 * @param kind	Kind to get name of.
 * @return		Kind name.
 */
cstring Edge::kindName(kind_t kind) {
	static cstring names[] = {
		"none",
		"taken",
		"not taken",
		"call",
		"virtual",
		"virtual call",
		"virtual return",
		"exception call",
		"exception return"
	};
	return names[kind];
}

static void print(Output& out, const Edge *edge) {
	out << edge->source() << " -> ";
	if(edge->kind() == Edge::CALL)
		out << "cfg(" << edge->calledCFG()->label() << ")";
	else
		out << edge->target();
	out << " (" << Edge::kindName(edge->kind()) << ")";
}

Output& operator<<(Output& out, const Edge *edge) {
	print(out, edge);
	return out;
}

Output& operator<<(Output& out, Edge *edge) {
	print(out, edge);
	return out;
}

} // otawa
