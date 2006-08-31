/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	EnhacedAggregationGraph.h -- EnhacedAggregationGraph class interface.
 */
#ifndef ENHACED_AGGREGATIONGRAPH_H
#define ENHACED_AGGREGATIONGRAPH_H

#include "AggregationGraph.h"

namespace otawa { namespace ipet {

class EnhacedAggregationGraph : public AggregationGraph{
protected:
	elm::Option<int> max_length;
	elm::Option<int> max_insts;
	elm::Option<int> max_joins;
	elm::Option<int> max_splits;
	void configure(PropList &props);
	virtual bool isBoundary(BBPath *cur_bbpath, BBPath *next_bbpath);
public:
	EnhacedAggregationGraph(CFG *cfg, PropList &props = PropList::EMPTY);
};

extern GenericIdentifier<int> Agg_Max_Length;
extern GenericIdentifier<int> Agg_Max_Insts;
extern GenericIdentifier<int> Agg_Max_Joins;
extern GenericIdentifier<int> Agg_Cur_Joins;
extern GenericIdentifier<int> Agg_Max_Splits;
extern GenericIdentifier<int> Agg_Cur_Splits;


} }

#endif /*ENHACED_AGGREGATIONGRAPH_H*/
