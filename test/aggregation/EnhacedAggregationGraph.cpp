/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	EnhacedAggregationGraph.cpp -- EnhacedAggregationGraph class implementation.
 */
#include "EnhacedAggregationGraph.h"

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class EnhacedAggregationGraph
 * This class extends from AggregationGraph and adds some additional
 * conditions to cut the paths, given in the properties.
 */


/**
 * Constructs a new EnhacedAggregationGraph
 * @param cfg source cfg from whose the graph will be made
 * @param props properties, including conditions on cutting
 */
EnhacedAggregationGraph::EnhacedAggregationGraph(CFG *cfg, PropList &props)
: AggregationGraph(cfg, props)
{
	configure(props);
}


bool EnhacedAggregationGraph::isBoundary(BBPath *cur_bbpath, BBPath *next_bbpath){
	// Inherit the annotation Agg_Cur_Joins and Agg_Cur_Splits from the cur_path
	int cur_joins = Agg_Cur_Joins(cur_bbpath);
	Agg_Cur_Joins(next_bbpath) = cur_joins;
	int cur_splits = Agg_Cur_Splits(cur_bbpath);
	Agg_Cur_Splits(next_bbpath) = cur_splits;

	bool bound = AggregationGraph::isBoundary(cur_bbpath, next_bbpath);
	BasicBlock *next_bb = next_bbpath->tail();
	if(max_length){
		bound = bound || next_bbpath->length() > *max_length;
	}
	if(max_insts){
		bound = bound || next_bbpath->countInsts() > *max_insts;
	}
	if(max_splits){
		BasicBlock *cur_bb = cur_bbpath->tail();
		int count_out = 0;
		for(BasicBlock::OutIterator edge(cur_bb); edge; edge++)
			count_out++;
		int cur_splits = Agg_Cur_Splits(next_bbpath);
		if(count_out > 1)
			cur_splits++;
		Agg_Cur_Splits(next_bbpath) = cur_splits;
		bound = bound || (cur_splits >= *max_splits);
	}
	if(max_joins){
		int count_in = 0;
		for(BasicBlock::InIterator edge(next_bb); edge; edge++)
			count_in++;
		int cur_joins = Agg_Cur_Joins(next_bbpath);
		if(count_in > 1)
			cur_joins++;
		Agg_Cur_Joins(next_bbpath) = cur_joins;
		bound = bound || (cur_joins >= *max_joins);
	}
	return bound;
}


void EnhacedAggregationGraph::configure(PropList& props){
	max_length = props.get<int>(Agg_Max_Length);
	max_insts = props.get<int>(Agg_Max_Insts);
	max_joins = props.get<int>(Agg_Max_Joins);
	max_splits = props.get<int>(Agg_Max_Splits);
}


/**
 * Limit for the number of basic blocks
 */
GenericIdentifier<int> Agg_Max_Length("otawa::ipet::agg_max_length");
/**
 * Limit for the maximum number of instructions by block
 */
GenericIdentifier<int> Agg_Max_Insts("otawa::ipet::agg_max_insts");
/**
 * Maximum joins of paths. number of joins needed to cut. 0 cut at every basic block
 */
GenericIdentifier<int> Agg_Max_Joins("otawa::ipet::agg_max_joins");
/**
 * Maximum path splitting
 */
GenericIdentifier<int> Agg_Max_Splits("otawa::ipet::agg_max_splits");




/**
 * Current number of joins for the current path
 */
GenericIdentifier<int> Agg_Cur_Joins("otawa::ipet::agg_cur_joins",0);
/**
 * Current number of splits for the current path
 */
GenericIdentifier<int> Agg_Cur_Splits("otawa::ipet::agg_cur_splits",0);





} }

