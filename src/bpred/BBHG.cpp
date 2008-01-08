#include "BBHG.h"
#include <iostream>
using namespace otawa;
using namespace elm;

/// BBHGEdge
BBHGEdge::BBHGEdge(BBHGNode *source, BBHGNode *target, bool taken, bool from_branch) :
				GenGraph<BBHGNode,BBHGEdge>::Edge(source,target) {
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
	for(BBHG::Successor s(this);s ;s++) {
		if(s->getCorrespondingBB()->number() == succ->getCorrespondingBB()->number()) {
			withT = withT || s.edge()->isTaken();
			withNT = withNT || !(s.edge()->isTaken());
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
