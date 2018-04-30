/*
 *	features for CCG plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_CCG_FEATURES_H_
#define OTAWA_CCG_FEATURES_H_

#include <elm/inhstruct/DLList.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/Inst.h>
#include "../graph/DiGraph.h"

namespace otawa {

class BasicBlock;
namespace ilp { class Var; }
namespace hard { class Cache; }

namespace ccg {

class Collection;
class Edge;
class LBlockSet;
class Node;


// LBlock class
class LBlock: public elm::inhstruct::DLNode, public PropList {
	friend class LBlockSet;
public:
	LBlock(LBlockSet *set, BasicBlock *bb, Inst *inst, t::uint32 size, int cache_index);
	inline int index(void) const { return idx; }
	inline Address address(void) const { return _inst->address(); }
	inline BasicBlock *bb(void) const { return _bb; }
	inline ot::size size(void) const { return _size; }
	//inline bool sameCacheBlock(const LBlock *block) const { return cacheBlock() == block->cacheBlock(); }
	inline LBlockSet *lblockset(void) const { return lbs; }
	int countInsts(void);
	inline int cacheBlock(void) const { return cid - 1; }
	inline Inst *instruction(void) const { return _inst; }

	// deprecated
	inline int number(void) const { return idx; }
	inline int id(void) { return idx; }
	inline int cacheblock(void) const { return cid; }

private:
	~LBlock(void);
	LBlockSet *lbs;
	Inst *_inst;
	t::uint32 _size;
	BasicBlock *_bb;
	int idx, cid;
};

Output& operator<<(Output& out, const LBlock *block);


class LBlockSet {
	friend class CCGDFA;
public:

	// Iterator class
	class Iterator:  public elm::Vector<LBlock *>::Iter {
	public:
		inline Iterator(LBlockSet& lbset):
			elm::Vector<LBlock *>::Iter(lbset.listelbc) { }
	};

	// Methods
	LBlockSet(int row, const hard::Cache *cache);
	int add(LBlock *node);
	inline int count(void) { return listelbc.length(); }
	inline int cacheBlockCount(void) { return listelbc.length(); }
	inline LBlock *lblock(int i) { return listelbc[i]; }
	inline int set(void) { return linenumber; }
	inline const hard::Cache *cache(void) const { return _cache; }

	// deprecated
	int line(void) { return linenumber; }

private:
	int linenumber;
	elm::Vector<LBlock *> listelbc;
	int cblock_count;
	const hard::Cache *_cache;
};


// Node class
class Node: public graph::GenVertex<Node, Edge> {
public:
	Node(LBlock *node);
	LBlock *lblock(){return lbl;};
private:
	LBlock *lbl;
};


// Edge class
class Edge: public PropList, public graph::GenEdge<Node, Edge> {
public:
	Edge(void);
	~Edge(void);
};


// Graph class
class Graph: public graph::GenDiGraph<Node, Edge> {
public:

	// Properties
	static Identifier<Node *> NODE;
	static Identifier<Collection *> GRAPHS;
};

// Features
extern p::feature COLLECTED_LBLOCKS_FEATURE;
extern Identifier<LBlockSet **> LBLOCKS;
extern Identifier<AllocArray<LBlock* >* > BB_LBLOCKS;

// CCG feature
extern p::feature FEATURE;

// constraint feature
extern Identifier<ilp::Var *> MISS_VAR;
extern p::feature CONSTRAINT_FEATURE;

} }		// otawa::ccg

#endif /* OTAWA_CCG_FEATURES_H_ */
