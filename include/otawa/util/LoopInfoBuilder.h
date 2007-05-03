/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/otawa/util/LoopInfo.h -- Loop Info Processor.
 */
#ifndef OTAWA_LOOP_INFO_BUILDER_H
#define OTAWA_LOOP_INFO_BUILDER_H

#include <assert.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/util/Dominance.h>
#include <otawa/properties.h>
#include <otawa/cfg.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {



extern Identifier<BasicBlock*> ENCLOSING_LOOP_HEADER;
extern Identifier<BasicBlock*> LOOP_EXIT_EDGE;
extern Identifier<elm::genstruct::Vector<Edge*> *> EXIT_LIST;


 
class LoopInfoBuilder: public CFGProcessor {
public:
        LoopInfoBuilder(void);
        virtual void processCFG(otawa::WorkSpace*, otawa::CFG*);
        
private:
		/**
		 * Builds the EXIT_LIST property for all the loop headers.
		 */
		void buildLoopExitList(otawa::CFG* cfg);
};

class LoopInfoProblem {
	class DominanceOrder {
 		public:
 		bool greaterThan(BasicBlock *bb1, BasicBlock *bb2) {
 			return(Dominance::dominates(bb1, bb2));
 		}
 	};
	CFG& _cfg;
	DominanceOrder order;
	genstruct::SortedSLList<BasicBlock *, DominanceOrder> headersLList;
	genstruct::Vector<BasicBlock *> hdrs;

 	public:
 	LoopInfoProblem(CFG& cfg);
 	inline dfa::BitSet *empty(void) const;
 	dfa::BitSet *gen(BasicBlock *bb) const;
 	dfa::BitSet *kill(BasicBlock *bb) const;
 	bool equals(dfa::BitSet *set1, dfa::BitSet *set2) const;
 	void reset(dfa::BitSet *set) const;
 	void merge(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void set(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void add(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void diff(dfa::BitSet *dst, dfa::BitSet *src);
 	inline int count(void) const;
 	inline BasicBlock *get(int index) const;
#ifndef NDEBUG
 	void dump(elm::io::Output& out, dfa::BitSet *set);
#endif 	
 	
};

extern Feature<LoopInfoBuilder> LOOP_INFO_FEATURE;

}	// otawa

#endif	// OTAWA_LOOP_INFO_H
