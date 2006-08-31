/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	AggregationGraph.h -- AggregationGraph class interface.
 */
#ifndef OTAWA_IPET_AGGREGATIONGRAPH_H
#define OTAWA_IPET_AGGREGATIONGRAPH_H

#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <elm/util/Pair.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace ipet {

class AggregationGraph: public CFG {
protected:
	CFG *cfg;
	elm::genstruct::Vector<BBPath*> elts;
	elm::genstruct::Vector<elm::Pair<BBPath*, BBPath*>*> edges;
	int length_of_longer_path;
	double mean_length;
	bool _made;
	
	void link(BBPath *src, BBPath *dst);
	void configure(PropList &props);
	virtual void makeGraph();
	virtual void scan();
	virtual bool isBoundary(BBPath *cur_bbpath, BBPath *next_bbpath);
	virtual bool isHeader(BBPath *cur_bbpath, BBPath *next_bbpath);
	static String pathName(BBPath *path);
	static String bbName(BasicBlock *bb);

public:
	AggregationGraph(CFG *cfg, PropList &props = PropList::EMPTY);
	virtual void toDot(elm::io::Output &out);
	virtual void printStats(elm::io::Output &out);
	virtual void printEquivalents(elm::io::Output &out);
};

} } // otawa::ipet

#endif /*OTAWA_IPET_AGGREGATIONGRAPH_H*/
