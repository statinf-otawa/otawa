/*
 *	$Id$
 *	BHG class interface
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
#ifndef BHG_H_
#define BHG_H_

#include <otawa/cfg.h>
#include <otawa/util/GenGraph.h>
#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/otawa.h>

namespace otawa { namespace bpred {

class BHGNode;
class BHGEdge;




class BHG : /*public otawa::PropList,*/ public otawa::GenGraph<BHGNode,BHGEdge> {
protected:
	friend class BHGNode;
	friend class BHGEdge;
	int m_history_size;
public:
	BHG(int history_size);
	int getClass();
	void add(BHGNode* node);
};




class BHGEdge : public otawa::GenGraph<BHGNode,BHGEdge>::GenEdge {
private:
	bool m_edge_taken;
public:
	BHGEdge(BHGNode *source, BHGNode *target, bool taken);
	~BHGEdge();
	bool isTaken();
};




class BHGNode : public otawa::GenGraph<BHGNode,BHGEdge>::GenNode{
private:
	bool m_entry;
	bool m_exit;
	bool m_exit_T; // exits with Taken
	bool m_exit_NT; // exits with NotTaken
	otawa::dfa::BitSet *m_history;
	otawa::BasicBlock *m_bb;
	int m_history_size;
public:
	BHGNode(otawa::BasicBlock* cfg_bb,const otawa::dfa::BitSet& bs, bool entry=false, bool exit=false, bool exit_T=false, bool exit_NT=false);
	~BHGNode();
	
	otawa::BasicBlock* getCorrespondingBB();
	otawa::dfa::BitSet& getHistory();
	bool isEntry();
	bool isExit();
	bool exitsWithT();
	bool exitsWithNT();
	bool isSuccessor(BHGNode* succ,bool& withT, bool& withNT);
	bool equals(const BHGNode& b);
	void setExit(bool withT = false, bool withNT = false);
};

} }		// otawa::bpred

#endif /*BHG_H_*/
