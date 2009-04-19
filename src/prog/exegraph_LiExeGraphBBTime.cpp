/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp --TrivialBBTime class implementation.
 */

#include <elm/io/OutFileStream.h>
#include <otawa/exegraph/LiExeGraphBBTime.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;
using namespace otawa::ipet;

#define DO_LOG
#if defined(NDEBUG) || !defined(DO_LOG)
#	define LOG(c)
#else
#	define LOG(c) c
#	define OUT	elm::cout
#endif

//#define ACCURATE_STATS
#if defined(NDEBUG) || !defined(ACCURATE_STATS)
#	define STAT(c)
#else
#	define STAT(c) c
#endif

namespace otawa {


LiExeGraphBBTime::LiExeGraphBBTime(const PropList& props)
:	ExeGraphBBTime<LiExeGraph>(props)
{
}


void LiExeGraphBBTime::buildPrologueList(
	BasicBlock * bb,
	ExeSequence<ExeNode> * prologue,
	int capacity,
	elm::genstruct::DLList<ExeSequence<ExeNode> *> * prologue_list) {
	// prologues are recursively built by considering preceeding nodes
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		BasicBlock * pred = edge->source();
		ExeSequence<ExeNode> * new_prologue = new ExeSequence<ExeNode>();
		for (InstIterator inst(prologue) ; inst ; inst++) {
			ExeInst<ExeNode> * eg_inst =
				new ExeInst<ExeNode>(inst->inst(), inst->basicBlock(), inst->codePart(), inst->index());
			new_prologue->addLast(eg_inst);
		}

		if (pred->countInstructions() == 0) {
			if (!new_prologue->isEmpty()) {
				// current sequence is terminated (not more instructions to add)
				// (end of recursive building)
				prologue_list->addLast(new_prologue);
			}
		}
		else {
		  // build new sequence from pred
		  elm::genstruct::DLList<Inst *> inst_list;
		  for(BasicBlock::InstIterator inst(pred); inst; inst++) {
		    inst_list.addLast(inst);
		  }
		  int index = 0;
		  if (!new_prologue->isEmpty())
		    index = new_prologue->first()->index() - 1;
		  while ( (new_prologue->count() < capacity) && (!inst_list.isEmpty()) ) {
		    ExeInst<ExeNode> * eg_inst =
				new ExeInst<ExeNode>(inst_list.last(), pred, LiExeGraph::PROLOGUE, index--);
		    new_prologue->addFirst(eg_inst);
		    inst_list.removeLast();
		  }
		  if (new_prologue->count() < capacity)
		    buildPrologueList(pred, new_prologue, capacity, prologue_list);
		  else {
		    prologue_list->addLast(new_prologue);
		  }
		}
	}

	delete prologue;

}


void LiExeGraphBBTime::buildEpilogueList(
			BasicBlock * bb,
			ExeSequence<ExeNode> * epilogue,
			int capacity,
			elm::genstruct::DLList<ExeSequence<ExeNode> *> * epilogue_list,
			int start_index) {
	// epilogues are recursively built by considering succeeding nodes
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		BasicBlock * succ = edge->target();
		ExeSequence<ExeNode> * new_epilogue = new ExeSequence<ExeNode>() ;
		for (InstIterator inst(epilogue) ; inst ; inst++) {
			ExeInst<ExeNode> * eg_inst =
				new ExeInst<ExeNode>(inst->inst(), inst->basicBlock(), inst->codePart(), inst->index());
			new_epilogue->addLast(eg_inst);
		}
		if (succ->countInstructions() == 0) {
			if (!new_epilogue->isEmpty()) {
				// current sequence is terminated (not more instructions to add)
				// (end of recursive building)
				epilogue_list->addLast(new_epilogue);
			}
		}
		else {
		  // build new sequence from succ
		  elm::genstruct::DLList<Inst *> inst_list;
		  for(BasicBlock::InstIterator inst(succ); inst; inst++) {
		    inst_list.addLast(inst);
		  }
		  int index;
		  if (!new_epilogue->isEmpty())
		    index = new_epilogue->last()->index() + 1;
		   else
		   	index = start_index;
		  while ( (new_epilogue->count() < capacity) && (!inst_list.isEmpty()) ) {
		    ExeInst<ExeNode> * eg_inst =
				new ExeInst<ExeNode>(inst_list.first(), succ, LiExeGraph::EPILOGUE, index++);
		    new_epilogue->addLast(eg_inst);
		    inst_list.removeFirst();
		  }
		  if (new_epilogue->count() < capacity)
		    buildEpilogueList(succ, new_epilogue, capacity, epilogue_list, new_epilogue->last()->index() + 1);
		  else {
		    epilogue_list->addLast(new_epilogue);

		  }
		}
	}
	delete epilogue;

}

int LiExeGraphBBTime::processSequence( WorkSpace *fw,
				ExeSequence<ExeNode> * prologue,
				ExeSequence<ExeNode> * body,
				ExeSequence<ExeNode> * epilogue,
				int capacity ) {

	ExeSequence<ExeNode> sequence;

	sequence.clear();
	if (prologue && (prologue->count() == capacity)) {
		int index = 0;
		index = prologue->first()->index() - 1;
		ExeInst<ExeNode> * eg_inst =
			new ExeInst<ExeNode>(NULL, NULL, LiExeGraph::BEFORE_PROLOGUE, index);
		sequence.addLast(eg_inst);
	}

	if (prologue) {
		for (InstIterator inst(prologue) ; inst ; inst++) {
			sequence.addLast(inst);
		}
	}
	for (InstIterator inst(body); inst ; inst++) {
		sequence.addLast(inst);
	}
	if (epilogue) {
		for (InstIterator inst(epilogue) ; inst ; inst++) {
			sequence.addLast(inst);
		}
	}

	// set dump file
	#ifdef DO_LOG
		elm::StringBuffer file_name;
		elm::String string_file_name, string_timed_file_name, extension, number, extension2;
		string_file_name = "./graphs/";
		string_timed_file_name = string_file_name;
		extension = ".dot";
		extension2 = "_times.dot";
		{
			LiExeGraph::code_part_t part = LiExeGraph::BEFORE_PROLOGUE;
			int bbnum = -1;
			file_name << body->first()->basicBlock()->number() << "+";
			for (InstIterator inst(&sequence) ; inst ; inst++) {
				if (inst->codePart() != part)
					file_name << "---";
				part = inst->codePart();
				if (inst->basicBlock()) {
					if (inst->basicBlock()->number() != bbnum)
						file_name << inst->basicBlock()->number() << "-";
					bbnum = inst->basicBlock()->number();
				}
			}
		}
		number = file_name.toString();
		string_file_name = string_file_name.concat(number);
		string_file_name = string_file_name.concat(extension);
		elm::io::OutFileStream dotStream(string_file_name);
		elm::io::Output dotFile(dotStream);
	#endif // DO_LOG

	LiExeGraph execution_graph(fw, microprocessor, &sequence);
	int bbExecTime = execution_graph.analyze();

	#ifdef DO_LOG
		string_timed_file_name = string_timed_file_name.concat(number);
		string_timed_file_name = string_timed_file_name.concat(extension2);
		elm::io::OutFileStream timedDotStream(string_timed_file_name);
		elm::io::Output timedDotFile(timedDotStream);
		// dump the execution graph *with times* in dot format
//		LOG(execution_graph.dump(timedDotFile));
	#endif // DO_LOG

	return bbExecTime;
}

void LiExeGraphBBTime::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
	ExeSequence<ExeNode> prologue, prefix, body, epilogue, sequence;

	LOG(OUT << "================================================================\n");
	LOG(OUT << "Processing block b" << bb->number() << ":\n\n");
	LOG(for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			OUT << inst->address() <<": ";
			inst->dump(OUT);
			OUT << "\n";
		}
	)

	if (bb->countInstructions() == 0)
		return;

	// compute prologue/epilogue size
	int capacity = 0;
	for(Microprocessor<ExeNode>::QueueIterator queue(microprocessor); queue; queue++){
		capacity += queue->size();
	}

	elm::genstruct::DLList<ExeSequence<ExeNode> *> prologue_list, epilogue_list;
	// build the list of body instructions
	int index = 1;
	for(BasicBlock::InstIterator inst(bb); inst; inst++) {
		ExeInst<ExeNode> *eg_inst =
			new ExeInst<ExeNode>(inst, bb, LiExeGraph::BODY, index++);
		body.addLast(eg_inst);
	}

	// build the list of possible prologues
	ExeSequence<ExeNode> * new_prologue = new ExeSequence<ExeNode>();
	buildPrologueList(bb, new_prologue, capacity, &prologue_list);

	// build the list of possible epilogues
//	ExeSequence<ExeNode> * new_epilogue = new ExeSequence<ExeNode>();
//	buildEpilogueList(bb, new_epilogue, capacity, &epilogue_list, body.count() + 1);
	int maxExecTime = 0;
	int bbExecTime;

	if (prologue_list.isEmpty()) {
		if (epilogue_list.isEmpty()) {
			bbExecTime = processSequence(fw, NULL, &body, NULL, capacity);
			if (bbExecTime > maxExecTime)
				maxExecTime = bbExecTime;
		}
		else {
			for (SeqIterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
				bbExecTime = processSequence(fw, NULL, &body, epilogue, capacity);
				if (bbExecTime > maxExecTime)
					maxExecTime = bbExecTime;
			}
		}
	}
	else {
		for (SeqIterator prologue(prologue_list) ; prologue ; prologue++) {
			if (epilogue_list.isEmpty()) {
				bbExecTime = processSequence(fw, prologue, &body, NULL, capacity);
				if (bbExecTime > maxExecTime)
					maxExecTime = bbExecTime;
			}
			else {
				for (SeqIterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
					bbExecTime = processSequence(fw, prologue, &body, epilogue, capacity);
					if (bbExecTime > maxExecTime)
						maxExecTime = bbExecTime;
				}
			}
		}
	}

	LOG(	OUT << "WCC of block " << bb->number() << " is " << maxExecTime << "\n";
		)
	TIME(bb) = maxExecTime;

}

/**
 * @class ExecutionGraphBBTime
 * This basic block processor computes the basic block execution time using
 * the execution graph method described in the following papers:
 * @li X. Li, A. Roychoudhury, A., T. Mitra, "Modeling Out-of-Order Processors
 * for Software Timing Analysis", Proceedings of the 25th IEEE International
 * Real-Time Systems Symposium (RTSS'04), 2004.
 * @li
 */


/**
 * This property is used to pass the microprocessor description to the
 * code processor. As default, it is extracted from the system description.
 */


/**
 * This property gives an output stream used to output log messages about
 * the execution of the algorithm.
 */
//GenericIdentifier<elm::io::Output *>  ExecutionGraphBBTime::LOG_OUTPUT("otawa.ExecutionGraphBBTime.log", &cerr);


/**
 * Build the ExecutionGraphBBTime processor.
 * @param props	Configuration properties possibly including @ref PROC and
 * @ref LOG.
 */



/**
 * Initialize statistics collection for the given basic block.
 * @param bb	Basic block to use.
 */


/**
 * Record statistics for the given sequence.
 * @param sequence	Sequence to use.
 * @param cost		Cost of the sequence.
 */


/**
 * Record the given node in the prefix tree.
 * @param parent	Parent node.
 * @param bb		Current BB.
 * @param cost		Cost of the current prefix.
 * @return			Node matching the given BB.
 */


/**
 * Record statistics for the given node.
 * @param node	Node to record stats for.
 * @param cost	Cost of the current prefix.
 */




/**
 * Call to end the statistics of the current basic block.
 */


/**
 * Record the time delta on the input edges of the current block.
 * @param insts	Prologue.
 * @param cost	Cost of the block with the current prologue.
 * @param bb	Current basic block.
 */



/**
 * If the statistics are activated, this property is returned in statistics
 * property list to store @ref ExecutionGraphBBTime statistics.
 */
//GenericIdentifier<Vector <stat_t> *>
//	EXEGRAPH_PREFIX_STATS("exegraph_prefix_stats", 0, OTAWA_NS);
//
//
///**
// * If set to the true, ExeGraph will also put @ref TIME_DELTA properties on
// * edges. Using the @ref TimeDetaObjectFunctionModifier, it allow to improve
// * the accuracy of the computed WCET.
// */
//GenericIdentifier<bool> EXEGRAPH_DELTA("exegraph_delta", false, OTAWA_NS);
//
//
///**
// * Set to true in the configuration @ref ExecutionGraphBBTime configure, this
// * processor will generate extra contraints and objects functions factor
// * taking in account the difference of execution time according prologues
// * of evaluated blocks.
// */
//GenericIdentifier<bool> EXEGRAPH_CONTEXT("exegraph_context", false, OTAWA_NS);
//

/**
 * Compute extra context to handle context.
 * @param cost	Cost of the BB.
 * @param stat	Statistics node.
 * @param ctx	Context previous node.
 */


//template ExecutionGraphBBTime<LiExeGraph>;

}  // otawa
