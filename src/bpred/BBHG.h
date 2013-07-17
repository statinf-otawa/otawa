/*
 *	$Id$
 *	BBHG class interface
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
#ifndef BBHG_H_
#define BBHG_H_
#include <otawa/util/GenGraph.h>
#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/otawa.h>
#include <otawa/cfg.h>

namespace otawa { namespace bpred {

class BBHGNode;
class BBHGEdge;




class BBHG : /*public otawa::PropList,*/ public otawa::GenGraph<BBHGNode,BBHGEdge> {
protected:
	friend class BHGNode;
	friend class BHGEdge;
	int m_history_size;
public:
	BBHG(int history_size);
	int getClass();
	void add(BBHGNode* node);
};




class BBHGEdge : public otawa::GenGraph<BBHGNode,BBHGEdge>::GenEdge {
private:
	bool m_edge_taken;
	bool m_from_branch;
public:
	BBHGEdge(BBHGNode *source, BBHGNode *target, bool taken=false, bool from_branch=false);
	~BBHGEdge();
	bool isTaken();
	bool isFromBranch();
};




class BBHGNode : public otawa::GenGraph<BBHGNode,BBHGEdge>::GenNode{
private:
	bool m_entry;
	bool m_exit;
	bool m_exit_T; // exits with Taken
	bool m_exit_NT; // exits with NotTaken
	bool m_branch;
	otawa::dfa::BitSet *m_history;
	otawa::BasicBlock *m_bb;
	int m_history_size;
public:
	BBHGNode(otawa::BasicBlock* cfg_bb,const otawa::dfa::BitSet& bs, bool branch=false, bool entry=false, bool exit=false, bool exit_T=false, bool exit_NT=false);
	~BBHGNode();
	
	otawa::BasicBlock* getCorrespondingBB();
	otawa::dfa::BitSet& getHistory();
	bool isEntry();
	bool isExit();
	bool exitsWithT();
	bool exitsWithNT();
	bool isBranch();
	bool isSuccessor(BBHGNode* succ,bool& withT, bool& withNT);
	bool equals(const BBHGNode& b);
	void setExit(bool isExit, bool withT = false, bool withNT = false);
};

} }		// otawa::bpred

#endif /*BBHG_H_*/
