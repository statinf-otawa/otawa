 /*	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/src/prog/CATNode.cpp -- implementation of CATNode class.
 */
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa {

CATNode::CATNode(LBlock *lblock ){
		lbl = lblock;
//		hasheaderevolution = false;						
 }

void CATNode::setHEADERLBLOCK(BasicBlock *hlb, bool loop){
	headerlblock = hlb;
	inloop = loop;
}
/*void CATNode::setHEADEREVOLUTION(BasicBlock *hev,bool evolution){
	headerevolution = hev;
	hasheaderevolution = evolution;
}*/
BasicBlock *CATNode::HEADERLBLOCK(void){
	return headerlblock;
}
/*BasicBlock *CATNode::HEADEREVOLUTION(void){
	return headerevolution;
}*/
bool CATNode::INLOOP (void){
	return inloop;	
}
/*bool CATNode::HASHEADEREVOLUTION(void){
	return hasheaderevolution;
}*/		
} // otawa
