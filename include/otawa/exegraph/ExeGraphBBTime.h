/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/TrivialBBTime.h -- TrivialBBTime class interface.
 */
#ifndef OTAWA_EXEGRAPHBBTIME_H
#define OTAWA_EXEGRAPHBBTIME_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/exegraph/ExecutionGraph.h>
#include <otawa/exegraph/Microprocessor.h>
#include <elm/inhstruct/DLList.h>
#include <elm/genstruct/DLList.h>

namespace otawa {
	
using namespace elm::genstruct; 
	
class PrefixCost {
	private:
		elm::genstruct::DLList<int> _costs;
		elm::genstruct::DLList<BasicBlock *> _reverse_prefix;
	public:
		inline PrefixCost(elm::genstruct::DLList<BasicBlock *> * prefix);
		inline ~PrefixCost();
		inline void addCost(int cost);
		inline elm::genstruct::DLList<BasicBlock *> * reversePrefix();
		inline bool isPrefix(elm::genstruct::DLList<BasicBlock *> * prefix);
		class CostIterator: public elm::genstruct::DLList<int>::Iterator {
				public:
					inline CostIterator(const PrefixCost *prefix_cost);
			};
		
};
 
inline PrefixCost::PrefixCost(elm::genstruct::DLList<BasicBlock *> * prefix) {
	if (prefix) {
		for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(*prefix) ; bb ; bb++) {
			_reverse_prefix.addLast(bb);
		}
	}
}

inline PrefixCost::~PrefixCost() {
	_costs.clear();
	_reverse_prefix.clear();
}

inline void PrefixCost::addCost(int cost) {
	_costs.addLast(cost);
}


inline elm::genstruct::DLList<BasicBlock *> * PrefixCost::reversePrefix() {
	return &_reverse_prefix;
}

inline bool PrefixCost::isPrefix(elm::genstruct::DLList<BasicBlock *> * prefix) {
	if (prefix->isEmpty()) {
		if (_reverse_prefix.isEmpty())
			return true;
		else
			return false;
	}
	else {
		if (_reverse_prefix.isEmpty())
			return false;
	}
	
	elm::genstruct::DLList<BasicBlock *>::Iterator bb1(*prefix), bb2(_reverse_prefix);
	bb1.first();
	bb2.first();
	while (!bb1.ended() && !bb2.ended()) {
		if (bb1.item() != bb2.item()) 
			return false;
		bb1.next();
		bb2.next();
	}
	if (bb1.ended() != bb2.ended())
		return false;
	return true;
}

inline PrefixCost::CostIterator::CostIterator(const PrefixCost *prefix_cost):
	elm::genstruct::DLList<int>::Iterator(prefix_cost->_costs) {
}


// --------------------------------------------------------------------
// Block sequence class
// --------------------------------------------------------------------

class BlockSequence {
	private:
		elm::genstruct::DLList<BasicBlock *> block_list;
		int _offset; // start point of the sequence (in the first block)
		int inst_count;
	public:
		inline BlockSequence();
		inline BlockSequence(BlockSequence *seq);
		inline void addBlockFirst(BasicBlock *bb, int off);
		inline void addBlockLast(BasicBlock *bb, int off);
		inline bool isEmpty(void);
		inline BasicBlock *first(void);
		inline BasicBlock *last(void);
		inline int instCount(void);
		inline elm::genstruct::DLList<BasicBlock *> * blockList(void);
		inline void dump(elm::io::Output& out_stream);
		inline int offset();
};

inline BlockSequence::BlockSequence() 
: inst_count(0) {
}

inline BlockSequence::BlockSequence(BlockSequence *seq) {
	for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(seq->block_list) ; bb ; bb++) {
		block_list.addLast(bb);
	}
	inst_count = seq->instCount();
}

inline void BlockSequence::addBlockFirst(BasicBlock *bb, int off) {
	block_list.addFirst(bb);
	_offset = off;
	inst_count = inst_count +  bb->countInstructions() - off;
}

inline void BlockSequence::addBlockLast(BasicBlock *bb, int off) {
	block_list.addLast(bb);
	_offset = off;
	inst_count = inst_count +  bb->countInstructions() - off;
}

inline bool BlockSequence::isEmpty(void) {
	return block_list.isEmpty();
}

inline BasicBlock * BlockSequence::first(void) {
	return block_list.first();
}

inline BasicBlock * BlockSequence::last(void) {
	return block_list.last();
}

inline int BlockSequence::instCount(void) {
	return inst_count;
}

inline elm::genstruct::DLList<BasicBlock *> * BlockSequence::blockList(void) {
	return &block_list;
}

inline void BlockSequence::dump(elm::io::Output& out_stream) {
	for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(this->block_list) ; bb ; bb++) {
		out_stream << "b" << bb->number();
		out_stream << " - ";
	}
	out_stream << " [offset=" << _offset << "] [inst_count=" << inst_count << "]";
}

inline int BlockSequence::offset() {
	return _offset;
}


// --------------------------------------------------------------------
// ExeGraphBBTime class
// --------------------------------------------------------------------

class ExeGraphBBTime: public BBProcessor {
	private:
		FrameWork *fw;
		PropList *properties;
		elm::io::Output& dumpFile;
		Microprocessor *microprocessor;
		bool delta, do_context;
		elm::genstruct::DLList<PrefixCost *> costs;
		
		
	public:
		ExeGraphBBTime(const PropList& props = PropList::EMPTY);
	
		void buildExecutionGraph(FrameWork *fw, 
								ExecutionGraph& graph, 
								elm::genstruct::DLList<Inst *> &prologue, 
								elm::genstruct::DLList<Inst *> &body, 
								elm::genstruct::DLList<Inst *> &epilogue);
		void buildPrologueList(	BasicBlock * bb,
								elm::genstruct::DLList<ExecutionGraphInstruction *> * prologue, 
								int capacity, 
								elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * prologue_list);
		void buildEpilogueList(BasicBlock * bb,
								elm::genstruct::DLList<ExecutionGraphInstruction *> * epilogue, 
								int capacity, 
								elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>* epilogue_list,
								int start_index);
		void buildPrefixList(
								BasicBlock * bb,
								elm::genstruct::DLList<ExecutionGraphInstruction *> * prefix, 
								int max_size, 
								int current_size,
								elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * prefix_list);
		int analyzeExecutionGraph(ExecutionGraph& graph);

		// BBProcessor overload
		void processFrameWork(FrameWork *fw);
		void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
		int processSequence( FrameWork *fw,
			elm::genstruct::DLList<ExecutionGraphInstruction *> * prologue,
			elm::genstruct::DLList<ExecutionGraphInstruction *> * body,
			elm::genstruct::DLList<ExecutionGraphInstruction *> * epilogue,
			int capacity ) ;

	// Configuration Properties
	static Identifier<Microprocessor *> PROCESSOR;
	static Identifier<elm::io::Output *>  LOG_OUTPUT;
	
	// statistics
	typedef struct stat_t {
		double total_span_sum;
		double total_vals_sum;
		int bb_cnt;
		int bb_span_sum;
		int bb_vals_sum;
		int seq_cnt;
		inline stat_t(void): total_span_sum(0), total_vals_sum(0),
		bb_cnt(0),bb_span_sum(0), bb_vals_sum(0), seq_cnt(0) { };
	} stat_t;

private:
		void recordDelta(
			DLList<ExecutionGraphInstruction *> *insts,
			int cost,
			BasicBlock *bb);

		typedef struct node_stat_t {
			struct node_stat_t *children, *sibling;
			int min, max;
			BasicBlock *bb;
			Vector<int> vals;
			inline node_stat_t(BasicBlock *_bb, int cost)
			: bb(_bb), children(0), sibling(0), min(cost), max(cost) { }
		} node_stat_t;

		Vector<stat_t>& exe_stats;
		node_stat_t stat_root;

		void initPrefixStats(BasicBlock *bb);		
		void recordPrefixStats(
			DLList<ExecutionGraphInstruction *> *insts,
			int cost);
		node_stat_t *recordPrefixNode(
			node_stat_t *parent,
			BasicBlock *bb,
			int cost);
		void recordPrefixNodeStats(node_stat_t *node, int cost);
		void collectPrefixStats(int depth = 0, node_stat_t *node = 0);
		
		typedef struct context_t {
			BasicBlock *bb;
			struct context_t *prev;
			inline context_t(BasicBlock *_bb, context_t *_prev)
			: bb(_bb), prev(_prev) { }
		} context_t;
		void buildContext(int cost, node_stat_t *stat = 0, context_t *ctx = 0); 
};

// Configuration
extern Identifier<bool> EXEGRAPH_DELTA;
extern Identifier<bool> EXEGRAPH_CONTEXT;

// Statistics output
extern Identifier<Vector <ExeGraphBBTime::stat_t> *> EXEGRAPH_PREFIX_STATS;

} //otawa

#endif // OTAWA_EXEGRAPHBBTIME_H
