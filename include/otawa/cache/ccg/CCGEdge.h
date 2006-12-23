/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/test/test_CCG/CCGEdge.h -- interface of CCGEdge class.
 */
#ifndef OTAWA_TEST_TESTCCG_CCGEDGE_H
#define OTAWA_TEST_TESTCCG_CCGEDGE_H

#include <otawa/ilp/Var.h>
#include <otawa/util/GenGraph.h>

namespace otawa {

// Classes
class CCGNode;
class Var;

// Edge class
class CCGEdge: public GenGraph<CCGNode,CCGEdge>::Edge {
	ilp::Var *p;
public:
	CCGEdge(CCGNode *source, CCGNode *target, ilp::Var *p1);
	~CCGEdge(void);
	ilp::Var *varEDGE(void);
};


} // otawa

#endif	// OTAWA_TEST_TESTCCG_CCGEDGE_H

