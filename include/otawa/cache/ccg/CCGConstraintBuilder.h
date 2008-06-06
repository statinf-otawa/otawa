/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CCGConstraintBuilder class interface
 */
#ifndef OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
#define OTAWA_CACHE_CCGCONSTRAINTBUILDER_H

#include <assert.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>
#include <otawa/dfa/BitSet.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg/BasicBlock.h>
#include <elm/util/Pair.h>


namespace otawa {


// Extern class
class CCGNode;
class CFG;
class ContextTree;
class LBlock;
class LBlockSet;
namespace ilp {
	class System;
	class Var;
}

namespace ipet {

extern Identifier<dfa::BitSet*> CCG_CONTEXTS;
// CCGConstraintBuilder class
class CCGConstraintBuilder: public Processor {
	bool _explicit;
	dfa::BitSet *ccg_contexts;
	HashTable<Pair<BasicBlock*, BasicBlock*>, Pair<dfa::BitSet*, int> > *confmap;
	
	void processLBlockSet(WorkSpace *fw, LBlockSet *lbset);
	void addConstraintHeader(ilp::System *system, LBlockSet *graph, ContextTree *ct,
		LBlock *boc);
public:
	CCGConstraintBuilder(void);
	
	// CFGProcessor overload
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void configure(const PropList& props = PropList::EMPTY);
};

} }	// otawa::ipet

#endif // OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
