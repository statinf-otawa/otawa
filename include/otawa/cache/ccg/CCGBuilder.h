/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	CCGBuilder class interface
 */
#ifndef OTAWA_CACHE_CCGBUILDER_H
#define OTAWA_CACHE_CCGBUILDER_H

#include <assert.h>
#include <elm/genstruct/Table.h>

#include <elm/genstruct/HashTable.h>
#include <otawa/proc/Processor.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/proc/Feature.h>
#include <otawa/util/GenGraph.h>
#include <otawa/dfa/BitSet.h>

namespace otawa {

using namespace elm::genstruct;
extern Identifier<HashTable<Pair<BasicBlock*,BasicBlock*>, Pair<dfa::BitSet*, int> > *> CCG_CONF_MAP;


// Extern classes
class LBlockSet;
class CFG;
class LBlockSet;
class CCGNode;
class CCGEdge;
class CCGCollection;

// CCG class
class CCG: public GenGraph<CCGNode, CCGEdge> {
public:
	
	// Properties
	static Identifier<CCGNode *> NODE;
	static Identifier<CCGCollection *> GRAPHS;
};


// CCGCollection class
class CCGCollection {
	friend class CCGBuilder;
	AllocatedTable<CCG *> ccgs;
	inline CCGCollection(int cnt): ccgs(cnt)
		{ for(int i = 0; i < cnt; i++) ccgs[i] = 0; }
public:
	inline ~CCGCollection(void)
		{ for(int i = 0; i < ccgs.count(); i++) if(ccgs[i]) delete ccgs[i]; }
	inline int length(void) const { return ccgs.count(); }
	inline CCG *get(int index) const { return ccgs[index]; }
	inline CCG *operator[](int index) const { return get(index); }
};


// CCGBuilder class
class CCGBuilder: public Processor {

	HashTable<Pair<BasicBlock*, BasicBlock*>, Pair<dfa::BitSet*, int> > *confmap;
	
	void processLBlockSet(WorkSpace *fw, LBlockSet *lbset);
public:
	static Identifier<bool> NON_CONFLICT;
	CCGBuilder(void);

	// CFGProcessor overload
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void configure(const PropList &props);
};

// Features
extern Feature<CCGBuilder> CCG_FEATURE;

}	// otawa

#endif // OTAWA_CACHE_CCGBUILDER_H
