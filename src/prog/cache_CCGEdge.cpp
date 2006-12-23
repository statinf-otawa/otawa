/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/test/test_CCG/CCGEdge.cpp -- implementation of CCGEdge class.
 */

#include <include/otawa/cfg.h>
#include <otawa/cache/ccg/CCGEdge.h>
#include <otawa/cache/ccg/CCGNode.h>

namespace otawa {

/**
 * @class CCGEdge
 * This class represents edges in the CCG representation.
 * They allow hooking annotations.
 */
 
/**
 * Build a new CCG edge.
 */
CCGEdge::CCGEdge(CCGNode *source, CCGNode *target, ilp::Var *p1) 
  : GenGraph<CCGNode,CCGEdge>::Edge(source,target)
{
	p = p1;
}

ilp::Var *CCGEdge::varEDGE(void){
	return p;	
}


/**
 * Delete an edge.
 */
CCGEdge::~CCGEdge(void) {
}

} // otawa
