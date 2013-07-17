/*
 *	$Id$
 *	BBHG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#include <otawa/platform.h>
#include "BBHG.h"
#include <iostream>
using namespace otawa;
using namespace elm;

namespace otawa { namespace bpred {

/// BBHGEdge
BBHGEdge::BBHGEdge(BBHGNode *source, BBHGNode *target, bool taken, bool from_branch) :
				GenGraph<BBHGNode,BBHGEdge>::GenEdge(source,target) {
	this->m_edge_taken = taken;
	this->m_from_branch = from_branch;
}


BBHGEdge::~BBHGEdge(void){
	
}

bool BBHGEdge::isTaken() {
	return this->m_edge_taken;
}

bool BBHGEdge::isFromBranch() {
	return this->m_from_branch;
}



/// BBHGNode
BBHGNode::BBHGNode(otawa::BasicBlock* cfg_bb,const otawa::dfa::BitSet &bs, bool branch,bool entry, bool exit,bool exit_T, bool exit_NT) {
	this->m_bb = cfg_bb;
	this->m_entry = entry;
	this->m_exit = exit;
	this->m_exit_T = exit_T;
	this->m_exit_NT = exit_NT;
	this->m_history = new otawa::dfa::BitSet(bs);
	this->m_history_size = bs.size();
	this->m_branch = branch;
}

BBHGNode::~BBHGNode() {
	delete this->m_history;
}

otawa::BasicBlock* BBHGNode::getCorrespondingBB() {
	return this->m_bb;
}

bool BBHGNode::isEntry() {
	return this->m_entry ;
}

bool BBHGNode::isExit() {
	return this->m_exit ;
}

bool BBHGNode::isBranch() {
	return this->m_branch;
}

bool BBHGNode::exitsWithT() {
	return this->m_exit_T;
}

bool BBHGNode::exitsWithNT() {
	return this->m_exit_NT;
}

bool BBHGNode::isSuccessor(BBHGNode* succ,bool& withT, bool& withNT) {
	withT = false;
	withNT = false;
	for(BBHG::OutIterator s(this); s ;s++) {
		if(s->target()->getCorrespondingBB()->number() == succ->getCorrespondingBB()->number()) {
			withT = withT || s->isTaken();
			withNT = withNT || !(s->isTaken());
		}
	}
	return (withT || withNT);
}

otawa::dfa::BitSet& BBHGNode::getHistory() {
	return  *(this->m_history);
}


bool BBHGNode::equals(const BBHGNode& b) {
	return (this->m_bb->number() == b.m_bb->number() && *(this->m_history) == *(b.m_history));
}

void BBHGNode::setExit(bool isExit,bool withT , bool withNT ) {
	this->m_exit_T = withT;
	this->m_exit_NT = withNT;
	
	this->m_exit = isExit || withT || withNT;
}


/// BBHG
BBHG::BBHG(int history_size) {
	this->m_history_size = history_size;
}

void BBHG::add(BBHGNode *node) {
	if(node->getHistory().size() == this->m_history_size) {
		otawa::GenGraph<BBHGNode,BBHGEdge>::add(node);		
	}
	else{
		cerr << "BBHG::add => history sizes don't match";
	}
}
int BBHG::getClass() {
	return this->m_history_size;
}

} }		// otawa::bpred

