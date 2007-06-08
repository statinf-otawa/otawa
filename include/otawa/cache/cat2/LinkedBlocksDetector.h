#ifndef CACHE_LINKEDBLOCKSDETECTOR_H_
#define CACHE_LINKEDBLOCKSDETECTOR_H_

#include <otawa/proc/Processor.h>

#include <otawa/cache/cat2/CAT2Builder.h>

namespace otawa {

extern Identifier<genstruct::Vector<LBlock*> *> LINKED_BLOCKS;

class NumberOrder {
	public:
	bool greaterThan(LBlock *lb1, LBlock *lb2) {
		return(CATEGORY_HEADER(lb1)->number() < CATEGORY_HEADER(lb2)->number());
	}
};

typedef genstruct::SortedSLList<LBlock*, NumberOrder> LinkedBlockList;

class LinkedBlocksDetector : public otawa::Processor {
	bool _explicit;
	NumberOrder order;
	void recordBlocks(Vector<LBlock*> *equiv);
	public:
	LinkedBlocksDetector(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList& props);
};

}

#endif
