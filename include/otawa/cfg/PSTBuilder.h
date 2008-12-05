/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *  Program Structure Tree Builder
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#ifndef CACHE_PSTBUILDER_H_
#define CACHE_PSTBUILDER_H_

#include <elm/assert.h>
#include <elm/util/Pair.h>
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/Tree.h>

#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {
	

/*
 * This represents a Cycle-Equivalence class: two edges are cycle-equivalent if each cycle passing thru an edge also passes thru the other.
 * (two edges are also cycle-equivalent if no cycle passes through the edges)
 */
class CEClass {		

	int count; // number of edges in this class
	bool first;
	
	Edge *backEdge; // backEdge associated with the class, if any. There is at most 1 backEdge per class (and at least 0).
	
	public:
	
	// Constructors
	inline CEClass(void) : count(0), first(true), backEdge(NULL) {
	}
	
	// Account for tree edges in this class
	inline void inc(void) {
		count++;
	}
	
	// Account for the backedge of this class, verifying that it is counted only once.
	inline void inc(Edge *bracket) {
		// Two back-edges cannot be cycle-equivalent.
		ASSERT((backEdge == NULL) || (backEdge == bracket));
		if (backEdge == NULL)
			count++;
		backEdge = bracket;
	}		
	inline void dec(void) {
		count--;
		first = false;
	}
	
	inline bool isLast(void) {
		return (count == 1);
	}
	
	inline bool isFirst(void) {
		return first;
	}
	
	inline int getCount(void) {
		return(count);
	}
	
	// Accessors
		
};

class SESERegion;

class SESERegion : public PropList {
	


	Edge *entry;
	Edge *exit;	
	CFG *cfg;	
	PSTree *parent;		
	Vector<BasicBlock*> bbs;
	bool first,last;
	CEClass *classe;	
	
	public:
	
	class BBIterator: public elm::genstruct::Vector<BasicBlock*>::Iterator {
		public:
		
	
		inline BBIterator(Vector<BasicBlock*> &_vec) :  elm::genstruct::Vector<BasicBlock*>::Iterator(_vec){
			
		}
		inline BBIterator(SESERegion *region) : elm::genstruct::Vector<BasicBlock*>::Iterator(region->bbs){
			
		}
	};
	inline SESERegion(CFG *_cfg, Edge *_entry, PSTree *_parent, bool _first, CEClass *_class) : entry(_entry), exit(NULL), cfg(_cfg),parent(_parent), first(_first), last(false), classe (_class) {
	}
	
	inline SESERegion(CFG *_cfg) : entry(NULL), exit(NULL), cfg(_cfg), parent(NULL),  classe(NULL) {
		BasicBlock::OutIterator outedge(cfg->entry());
		entry = *outedge;
		
		BasicBlock::InIterator inedge(cfg->exit());
		exit = *inedge;
	}
	
	inline PSTree *getParent() {
		return parent;
	}
	
	inline int countBB() {
		return(bbs.length());
	}
	inline void addBB(BasicBlock *_bb) {
		bbs.add(_bb);
	}
	inline void setExit(Edge *_exit) {
		exit = _exit;
	}
	inline void print() {
		cout << "Region: " << getEntry()->source()->number() << "->" << getEntry()->target()->number();
		if (getExit() != NULL) {
			cout << " to " << getExit()->source()->number() << "->" << getExit()->target()->number() ;
		} else cout << " to ???";		
	}
	inline Edge* getEntry() {
		return(entry);
	}
	inline Edge* getExit() {
		return(exit);
	}
	inline void setLast() {
		last = true;
	}
	inline bool isFirst() {
		return first;
	}
	CFG *getCFG() {
		return cfg;
	}
	const Vector<BasicBlock*> &getBBs() {
		return bbs;
	}
	inline bool isLast() {
		return last;
	}
	inline bool isRoot() {
		return (parent == NULL);
	}
	inline CEClass *getClass() {
		return classe;
	}
	
};



class PSTBuilder : public otawa::CFGProcessor {
	
	// Types
	typedef elm::Pair<Edge*,int> BSCName;
	typedef elm::genstruct::DLList<Edge*> BracketSet;
	


	
	// Properties
	static Identifier<int> PST_DFSNUM;
	static Identifier<int> PST_HI;
	static Identifier<BracketSet*> PST_BSET;
	
	static Identifier<CEClass*> PST_CLASS;
	static Identifier<int> PST_RECENTSIZE;
	static Identifier<CEClass*> PST_RECENTCLASS;
	
	static Identifier<bool> PST_IS_CAPPING;
	static Identifier<bool> DFS_IS_BACKEDGE;

	
	// Private members
	int cur_dfsnum;
	BasicBlock **node;
	Edge *fakeEdge;
	
	PSTree *pst;
	 
	// Private methods
	void depthFirstSearch(BasicBlock *bb);
	void assignClasses(CFG *cfg);
	void buildTree(CFG *cfg, BasicBlock *bb, PSTree *subtree);
	
	public:
	PSTBuilder(void);
	virtual void processCFG(WorkSpace *, CFG*);
	static VirtualCFG *getVCFG(PSTree *tree, HashTable<BasicBlock*,BasicBlock*> &map);
	static int displayTree(PSTree *node, int col = 0, bool ending = false);
		
};

}

#endif /*CACHE_PSTBUILDER_H_*/
