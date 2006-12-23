/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/test_CCG/CCGNode.cpp -- implementation of CCGNode class.
 */
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa {

CCGNode::CCGNode(LBlock *node){
		lbl = node;
 } 

} // otawa
