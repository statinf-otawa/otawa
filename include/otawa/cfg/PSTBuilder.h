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
#include <elm/data/BiDiList.h>
#include <elm/data/Tree.h>

#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {
	
/*
 * This represents a Cycle-Equivalence class: two edges are cycle-equivalent if each cycle passing through an edge also passes through the other.
 * (two edges are also cycle-equivalent if no cycle passes through the edges)
 */
class CEClass {		
public:
	
	inline CEClass(void) : count(0), first(true), backEdge(NULL) { }

	// Account for the backedge of this class, verifying that it is counted only once.
	inline void inc(Edge *bracket) {
		// Two back-edges cannot be cycle-equivalent.
		ASSERT((backEdge == NULL) || (backEdge == bracket));
		if (backEdge == NULL)
			count++;
		backEdge = bracket;
	}		

	inline void inc(void) { count++; }	// Account for tree edges in this class
	inline void dec(void) { count--; first = false; }
	inline bool isLast(void) { return (count == 1); }
	inline bool isFirst(void) { return first; }
	inline int getCount(void) { return(count); }
	
private:
	int count; // number of edges in this class
	bool first;
	Edge *backEdge; // backEdge associated with the class, if any. There is at most 1 backEdge per class (and at least 0).
};

class SESERegion: public PropList {
public:
	
	class BBIterator: public elm::Vector<Block*>::Iter {
	public:
		inline BBIterator(Vector<Block*> &_vec):  Vector<Block*>::Iter(_vec) { }
		inline BBIterator(SESERegion *region): Vector<Block*>::Iter(region->bbs) { }
	};
	
	inline SESERegion(CFG *_cfg, Edge *_entry, PSTree *_parent, bool _first, CEClass *_class)
		: entry(_entry), exit(NULL), cfg(_cfg),parent(_parent), first(_first), last(false), classe (_class) { }
	inline SESERegion(CFG *_cfg): entry(NULL), exit(NULL), cfg(_cfg), parent(NULL), first(0), last(0), classe(NULL) {
		Block::EdgeIter outedge = cfg->entry()->outs();
		entry = *outedge;
		Block::EdgeIter inedge = cfg->exit()->ins();
		exit = *inedge;
	}
	
	inline PSTree *getParent(void) { return parent; }
	inline int countBB(void) { return(bbs.length()); }
	inline void addBB(Block *_bb) { bbs.add(_bb); }
	inline void setExit(Edge *_exit) { exit = _exit; }
	inline void print() {
		cout << "Region: " << getEntry()->source() << "->" << getEntry()->target();
		if (getExit() != NULL)
			cout << " to " << getExit()->source() << "->" << getExit()->target() ;
		else
			cout << " to ???";
	}
	inline Edge* getEntry(void) { return(entry); }
	inline Edge* getExit(void) { return(exit); }
	inline void setLast(void) { last = true; }
	inline bool isFirst(void) { return first; }
	CFG *getCFG() { return cfg; }
	const Vector<Block*> &getBBs() { return bbs; }
	inline bool isLast(void) { return last; }
	inline bool isRoot(void) { return (parent == NULL); }
	inline CEClass *getClass() { return classe; }

private:
	Edge *entry;
	Edge *exit;
	CFG *cfg;
	PSTree *parent;
	Vector<Block *> bbs;
	bool first,last;
	CEClass *classe;
};


class PSTBuilder : public otawa::CFGProcessor {
	typedef elm::Pair<Edge *, int> BSCName;
	typedef elm::BiDiList<Edge *> BracketSet;

public:
	PSTBuilder(void);
	virtual void processCFG(WorkSpace *, CFG*);
	//static VirtualCFG *getVCFG(PSTree *tree, HashTable<BasicBlock*, BasicBlock*> &map);
	static int displayTree(PSTree *node, int col = 0, bool ending = false);

private:
	// Properties
	static Identifier<int> PST_DFSNUM;
	static Identifier<int> PST_HI;
	static Identifier<BracketSet *> PST_BSET;
	
	static Identifier<CEClass *> PST_CLASS;
	static Identifier<int> PST_RECENTSIZE;
	static Identifier<CEClass *> PST_RECENTCLASS;
	
	static Identifier<bool> PST_IS_CAPPING;
	static Identifier<bool> DFS_IS_BACKEDGE;

	
	// Private members
	int cur_dfsnum;
	Block **node;
	//Edge *fakeEdge;
	PSTree *pst;
	 
	// Private methods
	void depthFirstSearch(Block *bb);
	void assignClasses(CFG *cfg);
	void buildTree(CFG *cfg, Block *bb, PSTree *subtree);
	
};

}

#endif /*CACHE_PSTBUILDER_H_*/
