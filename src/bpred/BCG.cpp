/*
 *	$Id$
 *	BCG class implementation
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
#include "BCG.h"
using namespace otawa;

namespace otawa { namespace bpred {

/// BCGEdge
BCGEdge::BCGEdge(BCGNode *source, BCGNode *target, bool taken) :
				GenGraph<BCGNode,BCGEdge>::GenEdge(source,target) {
	this->m_edge_taken = taken;
	
}


BCGEdge::~BCGEdge(void){
	
}

bool BCGEdge::isTaken() {
	return this->m_edge_taken;
}


/// BCGNode
BCGNode::BCGNode(int cfg_bb,bool entry, bool exit,const otawa::dfa::BitSet *history,bool exit_T, bool exit_NT) {
	this->m_bb = cfg_bb;
	this->m_entry = entry;
	this->m_exit = exit;
	this->m_exit_T = exit_T;
	this->m_exit_NT = exit_NT;
	this->m_history= NULL;
	if(history!=NULL) this->m_history = new otawa::dfa::BitSet(*history);
}

BCGNode::BCGNode(int cfg_bb,bool entry, bool exit,bool exit_T, bool exit_NT) {
	this->m_bb = cfg_bb;
	this->m_entry = entry;
	this->m_exit = exit;
	this->m_exit_T = exit_T;
	this->m_exit_NT = exit_NT;
	this->m_history = NULL;
}

BCGNode::~BCGNode() {
	if(this->m_history!= NULL) delete this->m_history;
}

int BCGNode::getCorrespondingBBNumber() {
	return this->m_bb;
}

bool BCGNode::isEntry() {
	return this->m_entry ;
}

bool BCGNode::isExit() {
	return this->m_exit ;
}

bool BCGNode::exitsWithT() {
	return this->m_exit_T;
}

bool BCGNode::exitsWithNT() {
	return this->m_exit_NT;
}

bool BCGNode::isSuccessor(BCGNode* succ,bool& withT, bool& withNT) {
	withT = false;
	withNT = false;
	for(BCG::OutIterator s(this);s ;s++) {
		if(s->target()->getCorrespondingBBNumber() == succ->getCorrespondingBBNumber()) {
			withT = withT || s->isTaken();
			withNT = withNT || !(s->isTaken());
		}
	}
	return (withT || withNT);
}
otawa::dfa::BitSet& BCGNode::getHistory() {
	return *(this->m_history);
}
/// BCG
BCG::BCG(int _class) {
	this->m_class = _class;
	this->m_history  = NULL;
}
BCG::BCG(const otawa::dfa::BitSet& _hist) {
	this->m_history = new otawa::dfa::BitSet(_hist);
}
BCG::~BCG() {
	if (this->m_history!=NULL) delete this->m_history;
}
int BCG::getClass() {
	return this->m_class;
}
otawa::dfa::BitSet& BCG::getHistory() {
	return *(this->m_history);
}

} }		// otawa::bpred
