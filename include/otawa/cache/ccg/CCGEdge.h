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
class CCGEdge: public GenGraph<CCGNode,CCGEdge>::Edge, public PropList {
public:
	CCGEdge(CCGNode *source, CCGNode *target);
	~CCGEdge(void);
};


} // otawa

#endif	// OTAWA_TEST_TESTCCG_CCGEDGE_H

