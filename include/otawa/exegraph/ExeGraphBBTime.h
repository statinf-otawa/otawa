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

// --------------------------------------------------------------------
// Block sequence class
// --------------------------------------------------------------------

class BlockSequence {
	private:
		elm::genstruct::DLList<BasicBlock *> block_list;
		int offset; // start point of the sequence (in the first block)
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
	offset = off;
	inst_count = inst_count +  bb->countInstructions() - off;
}

inline void BlockSequence::addBlockLast(BasicBlock *bb, int off) {
	block_list.addLast(bb);
	offset = off;
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
	out_stream << " [offset=" << offset << "] [inst_count=" << inst_count << "]";
}


// --------------------------------------------------------------------
// ExeGraphBBTime class
// --------------------------------------------------------------------

class ExeGraphBBTime: public BBProcessor {
	private:
		PropList *properties;
		elm::io::Output& dumpFile;
		Microprocessor *microprocessor;
		
		
	public:
		inline ExeGraphBBTime(const PropList& props = PropList::EMPTY, 
								Microprocessor *processor = NULL, 
								elm::io::Output& out_stream = elm::cout);
	
		void buildExecutionGraph(FrameWork *fw, 
								ExecutionGraph& graph, 
								elm::genstruct::DLList<Inst *> &prologue, 
								elm::genstruct::DLList<Inst *> &body, 
								elm::genstruct::DLList<Inst *> &epilogue);
		void buildPrologueList(BlockSequence *sequence, 
								int capacity, 
								elm::genstruct::DLList<BlockSequence *>& prologue_list);
		void buildEpilogueList(BlockSequence *sequence, 
								int capacity, 
								elm::genstruct::DLList<BlockSequence *>& epilogue_list);
		int analyzeExecutionGraph(ExecutionGraph& graph);
		// BBProcessor overload
		void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

inline ExeGraphBBTime::ExeGraphBBTime(const PropList& props, 
									Microprocessor *processor, 
									elm::io::Output& out_stream) 
: BBProcessor(props), microprocessor(processor), dumpFile(out_stream) {
}


} //otawa

#endif // OTAWA_EXEGRAPHBBTIME_H
