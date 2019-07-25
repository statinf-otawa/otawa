/*
 *	$Id$
 *	BHG class implementation
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

#include "BHG.h"
//#include <iostream>
using namespace otawa;

namespace otawa { namespace bpred {

/// BHGEdge
BHGEdge::BHGEdge(bool taken) {
	this->m_edge_taken = taken;
	
}


BHGEdge::~BHGEdge(void){
	
}

bool BHGEdge::isTaken() {
	return this->m_edge_taken;
}





/// BHGNode
BHGNode::BHGNode(otawa::Block* cfg_bb,const otawa::dfa::BitSet &bs,bool entry, bool exit,bool exit_T, bool exit_NT) {
	this->m_bb = cfg_bb;
	this->m_entry = entry;
	this->m_exit = exit;
	this->m_exit_T = exit_T;
	this->m_exit_NT = exit_NT;
	this->m_history = new otawa::dfa::BitSet(bs);
	this->m_history_size = bs.size();
}

BHGNode::~BHGNode() {
	delete this->m_history;
}

otawa::Block* BHGNode::getCorrespondingBB() {
	return this->m_bb;
}

bool BHGNode::isEntry() {
	return this->m_entry ;
}

bool BHGNode::isExit() {
	return this->m_exit ;
}

bool BHGNode::exitsWithT() {
	return this->m_exit_T;
}

bool BHGNode::exitsWithNT() {
	return this->m_exit_NT;
}

bool BHGNode::isSuccessor(BHGNode* succ,bool& withT, bool& withNT) {
	withT = false;
	withNT = false;
	for(auto s = outs(); s() ;s++) {
		if(s->sink()->getCorrespondingBB()->index() == succ->getCorrespondingBB()->index()) {
			withT = withT || s->isTaken();
			withNT = withNT || !(s->isTaken());
		}
	}
	return (withT || withNT);
}

otawa::dfa::BitSet& BHGNode::getHistory() {
	return  *(this->m_history);
}


bool BHGNode::equals(const BHGNode& b) {
	return (this->m_bb->index() == b.m_bb->index() && *(this->m_history) == *(b.m_history));
}

void BHGNode::setExit(bool withT , bool withNT ) {
	this->m_exit_T = withT;
	this->m_exit_NT = withNT;
	
	this->m_exit = withT || withNT;
}


/// BHG
BHG::BHG(int history_size) {
	this->m_history_size = history_size;
}

void BHG::add(BHGNode *node) {
	// TODO
	/*if(node->getHistory().size() == this->m_history_size) {
		otawa::ograph::GenGraph<BHGNode,BHGEdge>::add(node);
	}
	else {
		cerr << "BHG::add => history sizes don't match";

	}*/
}
int BHG::getClass() {
	return this->m_history_size;
}

} }		// otawa::bpred
