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
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/util/GenGraph.h>

namespace otawa {

using namespace elm::genstruct;

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
	static GenericIdentifier<CCGNode *> NODE;
	static GenericIdentifier<CCGCollection *> GRAPHS;
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
	void processLBlockSet(FrameWork *fw, LBlockSet *lbset);
public:
	static GenericIdentifier<bool> NON_CONFLICT;
	CCGBuilder(void);

	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
};

// Features
extern Feature<CCGBuilder> CCG_FEATURE;

}	// otawa

#endif // OTAWA_CACHE_CCGBUILDER_H
