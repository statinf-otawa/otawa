/*
 * DataCatBuilder.h
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#ifndef DATACATBUILDER_H_
#define DATACATBUILDER_H_

#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/dcache/BlockBuilder.h>
#include <otawa/dcache/ACSBuilder.h>

namespace otawa { namespace dcache {


extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock *> CATEGORY_HEADER;

// CATBuilder class
class CATBuilder: public Processor {
public:
	CATBuilder(void);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void configure(const PropList &props);
	virtual void cleanup(WorkSpace *ws);

private:
	void processLBlockSet(WorkSpace *ws, const otawa::BlockCollection& coll, const hard::Cache *cache);
	data_fmlevel_t firstmiss_level;
	//CategoryStats *cstats;
};

// feature
extern Feature<CATBuilder> CACHE_CATEGORY_FEATURE;

} }	// otawa::dcache

#endif /* DATACATBUILDER_H_ */
