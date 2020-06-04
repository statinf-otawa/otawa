/*
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

#include <elm/data/FragTable.h>
#include <otawa/cfg/CFG.h>
#include <otawa/proc/Feature.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/util/Bag.h>

namespace elm { template <class T> class Tree; }

namespace otawa {

// Pre-declarations
class CFGCollector;
class CFGInfo;
class LoopUnroller;
class SESERegion;
typedef elm::Tree<SESERegion*> PSTree;
namespace graph { class Edge; }

// CFGCollection Class
class CFGCollection {
public:
	static const CFGCollection *get(WorkSpace *ws);
	inline int count(void) const { return cfgs.length(); }
	inline CFG *get(int index) const { return cfgs[index]; }
	inline CFG *operator[](int index) const { return cfgs[index]; }
	inline CFG *entry(void) const { return get(0); }
	int countBlocks(void) const;

	class Iter: public FragTable<CFG *>::Iter {
		friend class CFGCollection;
	public:
		inline Iter(void) { }
		inline Iter(const CFGCollection *cfgs): elm::FragTable<CFG *>::Iter(cfgs->cfgs) { }
		inline Iter(const CFGCollection& cfgs): elm::FragTable<CFG *>::Iter(cfgs.cfgs) { }
	private:
		inline Iter(const FragTable<CFG *>::Iter& i): FragTable<CFG *>::Iter(i) { }
	};

	inline Iter items(void) const { return Iter(this); }
	inline Iter operator*(void) const { return items(); }
	inline Iter begin(void) const { return items(); }
	inline Iter end(void) const { return cfgs.end(); }

	class BlockIter: public PreIterator<BlockIter, Block *> {
		friend class CFGCollection;
	public:
		inline BlockIter(void) { }
		inline BlockIter(const CFGCollection *cfgs): cfg(cfgs), bb(cfg->blocks()) { }
		inline BlockIter(const CFGCollection& cfgs): cfg(&cfgs), bb(cfg->blocks()) { }
		inline bool ended(void) const { return bb.ended(); }
		inline Block *item(void) const { return *bb; }
		inline void next(void) { bb++; if(!bb) { cfg++; if(cfg()) bb = CFG::BlockIter(cfg->blocks()); } }
		inline bool equals(const BlockIter& i) const { return cfg.equals(i.cfg) && bb.equals(bb); }
	private:
		inline BlockIter(const CFGCollection& coll, int): cfg(coll.end()) { }
		Iter cfg;
		CFG::BlockIter bb;
	};

	class BlockRange {
	public:
		inline BlockRange(const CFGCollection& coll): _coll(coll) { }
		inline BlockIter begin(void) const { return BlockIter(_coll); }
		inline BlockIter end(void) const { return BlockIter(_coll, 0); }
	private:
		const CFGCollection& _coll;
	};
	inline BlockRange blocks(void) const { return BlockRange(*this); }

	void add(CFG *cfg);

private:
	elm::FragTable<CFG *> cfgs;
};

// context support
extern p::id<ContextualPath> CONTEXT;
extern p::id<ContextualStep> ENTER;
extern p::id<int> LEAVE;

// COLLECTED_CFG_FEATURE
extern p::id<CFG *> ENTRY_CFG;
extern p::id<Bag<Address> > BB_BOUNDS;
extern p::id<Address> ADDED_CFG;
extern p::id<CString> ADDED_FUNCTION;
extern p::interfaced_feature<const CFGCollection> COLLECTED_CFG_FEATURE;
extern p::id<const CFGCollection *> INVOLVED_CFGS;
extern p::id<Edge *> CALLED_BY;

// CFGInfoFeature
extern p::feature CFG_INFO_FEATURE;
extern Identifier<const CFGInfo *> CFG_INFO;

// REDUCED_LOOPS_FEATURE
extern p::feature REDUCED_LOOPS_FEATURE;

// UNROLLED_LOOPS_FEATURE
extern p::id<bool> UNROLL_THIS;
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
extern Identifier<elm::Vector<Edge*> *> EXIT_LIST;
extern p::feature LOOP_INFO_FEATURE;
class LoopIter: public PreIterator<LoopIter, Block *> {
public:
	inline LoopIter(void): h(0) { }
	inline LoopIter(Block *b): h(LOOP_HEADER(b) ? b : ENCLOSING_LOOP_HEADER(b)) { }
	inline bool ended(void) const { return !h; }
	inline Block *item(void) const { return h; }
	inline void next(void) { h = ENCLOSING_LOOP_HEADER(h); }
private:
	Block *h;
};

// CONDITIONAL_RESTRUCTURED_FEATURE
extern p::feature CONDITIONAL_RESTRUCTURED_FEATURE;

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
	virtual Block* idom(Block* b) = 0;
	virtual bool isBackEdge(Edge *edge) = 0;

	inline bool dominates(Block *b1, Block *b2) { return dom(b1, b2); }
	inline bool isDominated(Block *b1, Block *b2) { return dominates(b2, b1); }
};
extern p::interfaced_feature<DomInfo> DOMINANCE_FEATURE;

// Post-domination
class PostDomInfo {
public:
	virtual ~PostDomInfo(void) = 0;
	virtual bool pdom(Block *b1, Block *b2) = 0;
};
namespace dfa { class BitSet; }
extern p::interfaced_feature<PostDomInfo> POSTDOMINANCE_FEATURE;

// Loop support
class Loop;
class LoopManager {
public:
	virtual ~LoopManager();
	virtual Loop *top(CFG *cfg) = 0;
	virtual Loop *loop(Block *v) = 0;
};
extern p::interfaced_feature<LoopManager *> EXTENDED_LOOP_FEATURE;

// Split CFG support
typedef Pair<BasicBlock *, Inst *> location_t;
extern p::feature SPLIT_CFG;
extern p::id<Address> CFG_START;
extern p::id<Address> CFG_STOP;
extern p::id<location_t> LOCATION_START;
extern p::id<location_t> LOCATION_STOP;
io::Output& operator<<(io::Output& out, const location_t& loc);

// CFG normalization
extern p::feature NORMALIZED_CFGS_FEATURE;

} // otawa

#endif /* OTAWA_CFG_FEATURES_H_ */
