/*
 *	CATNode class interface
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#include <otawa/cat/CATNode.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa { namespace cat {

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
} }	// otawa::cat
