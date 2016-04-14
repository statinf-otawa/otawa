/*
 *	$Id$
 *	features of module cfg
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
#ifndef OTAWA_CFG_FEATURES_H_
#define OTAWA_CFG_FEATURES_H_

#include <elm/genstruct/FragTable.h>
#include <otawa/cfg/CFG.h>
#include <otawa/proc/Feature.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/util/Bag.h>

namespace elm { namespace genstruct { template <class T> class Tree; } }

namespace otawa {

// Pre-declarations
class BasicBlock;
class CFG;
class CFGCollector;
class CFGInfo;
class Edge;
class LoopUnroller;
class SESERegion;
typedef elm::genstruct::Tree<SESERegion*> PSTree;

// CFGCollection Class
class CFGCollection {
public:
	inline int count(void) const { return cfgs.length(); }
	inline CFG *get(int index) const { return cfgs[index]; }
	inline CFG *operator[](int index) const { return cfgs[index]; }
	inline CFG *entry(void) const { return get(0); }
	int countBB(void) const;

	class Iterator: public elm::genstruct::FragTable<CFG *>::Iterator {
	public:
		inline Iterator(const CFGCollection *cfgs)
			: elm::genstruct::FragTable<CFG *>::Iterator(cfgs->cfgs) { }
		inline Iterator(const CFGCollection& cfgs)
			: elm::genstruct::FragTable<CFG *>::Iterator(cfgs.cfgs) { }
	};

	class BBIterator: public PreIterator<BBIterator, BasicBlock *> {
	public:
		inline BBIterator(const CFGCollection *cfgs): cfg(cfgs), bb(cfg->blocks()) { }
		inline BBIterator(const BBIterator& i): cfg(i.cfg), bb(i.bb) { }
		inline BBIterator& operator=(const BBIterator& i) { cfg = i.cfg; bb = i.bb; return *this; }
		inline bool ended(void) const { return bb.ended(); }
		inline Block *item(void) const { return *bb; }
		inline void next(void) { bb++; if(!bb) { cfg++; if(cfg) bb = CFG::BlockIter(cfg->blocks()); } }
	private:
		Iterator cfg;
		CFG::BlockIter bb;
	};

	void add(CFG *cfg);

private:
	elm::genstruct::FragTable<CFG *> cfgs;
};

// context support
extern Identifier<ContextualPath> CONTEXT;
extern Identifier<ContextualStep> ENTER;
extern Identifier<int> LEAVE;

// COLLECTED_CFG_FEATURE
extern Identifier<CFG *> ENTRY_CFG;
extern Identifier<Bag<Address> > BB_BOUNDS;
extern Identifier<Address> ADDED_CFG;
extern Identifier<CString> ADDED_FUNCTION;
extern p::feature COLLECTED_CFG_FEATURE;
extern Identifier<const CFGCollection *> INVOLVED_CFGS;
extern Identifier<Edge *> CALLED_BY;

// CFGInfoFeature
extern p::feature CFG_INFO_FEATURE;
extern Identifier<const CFGInfo *> CFG_INFO;

// REDUCED_LOOPS_FEATURE
extern p::feature REDUCED_LOOPS_FEATURE;

// UNROLLED_LOOPS_FEATURE
extern p::feature UNROLLED_LOOPS_FEATURE;
extern Identifier<Block*> UNROLLED_FROM;

// VIRTUALIZED_CFG_FEATURE
extern Identifier<bool> VIRTUAL_DEFAULT;
extern Identifier<bool> NO_INLINE;
extern Identifier<bool> INLINING_POLICY;
extern p::feature VIRTUALIZED_CFG_FEATURE;
extern Identifier<bool> RECURSIVE_CALL;

// CFG_CHECKSUM_FEATURE
extern p::feature CFG_CHECKSUM_FEATURE;
extern Identifier<unsigned long > CHECKSUM;

// CHECKED_CFG_FEATURE
extern p::feature CHECKED_CFG_FEATURE;

// DELAYED_CFG_FEATURE
extern p::feature DELAYED_CFG_FEATURE;
extern Identifier<bool> DELAYED_INST;
extern Identifier<bool> DELAYED_NOP;

// LOOP_HEADERS_FEATURE
extern p::feature LOOP_HEADERS_FEATURE;
extern Identifier<bool> LOOP_HEADER;
extern Identifier<bool> BACK_EDGE;

// LOOP_INFO_FEATURE
extern Identifier<Block*> ENCLOSING_LOOP_HEADER;
extern Identifier<Block*> LOOP_EXIT_EDGE;
extern Identifier<elm::genstruct::Vector<Edge*> *> EXIT_LIST;
extern p::feature LOOP_INFO_FEATURE;

// Loop Header Iterator
class LoopHeaderIter: public PreIterator<LoopHeaderIter, Block*> {
public:
	inline LoopHeaderIter(Block* b): lh(LOOP_HEADER(b) ? b : ENCLOSING_LOOP_HEADER.get(lh, NULL)) { }
	inline LoopHeaderIter(const LoopHeaderIter& i): lh(i.lh) { }
	inline LoopHeaderIter& operator=(const LoopHeaderIter& i) { lh = i.lh; return *this; }

	inline bool ended(void) const { return lh == NULL; }
	Block* item(void) const { return lh; }
	inline void next(void) { lh = ENCLOSING_LOOP_HEADER.get(lh, NULL); }

private:
	Block* lh; // can be NULL
};

// CFG I/O service
class CFGSaver;
class CFGLoader;
extern Identifier<sys::Path> CFG_IO_PATH;

// WEIGHT_FEATURE
extern Identifier<int> WEIGHT;
extern p::feature WEIGHT_FEATURE;

// Dominance feature
class DomInfo {
public:
	virtual ~DomInfo(void);
	virtual bool dom(Block *b1, Block *b2) = 0;
	virtual bool isBackEdge(Edge *edge) = 0;

	inline bool dominates(Block *b1, Block *b2) { return dom(b1, b2); }
	inline bool isDominated(Block *b1, Block *b2) { return dominates(b2, b1); }
};
extern p::feature DOMINANCE_FEATURE;
extern Identifier<DomInfo *> DOM_INFO;

// Post-domination
extern p::feature POSTDOMINANCE_FEATURE;
class PostDomInfo {
public:
	virtual ~PostDomInfo(void) = 0;
	virtual bool pdom(Block *b1, Block *b2) = 0;
};
namespace dfa { class BitSet; }
extern Identifier<PostDomInfo *> PDOM_INFO;

} // otawa

#endif /* OTAWA_CFG_FEATURES_H_ */
