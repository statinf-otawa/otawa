/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp --TrivialBBTime class implementation.
 */

#include <stdlib.h>
#include <otawa/otawa.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ipet/IPET.h>
#include <elm/genstruct/Vector.h>
#include <otawa/exegraph/ExecutionGraph.h>
#include <elm/genstruct/Table.h>
#include <elm/genstruct/DLList.h>
#include <otawa/hard/Register.h>
#include <otawa/exegraph/ExeGraphBBTime.h>

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
#	define OUT	dumpFile
#endif

namespace otawa { 

/**
 * @class ExeGraphBBTime
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
GenericIdentifier<Microprocessor *> ExeGraphBBTime::PROCESSOR("otawa.ExeGraphBBTime.proc.", NULL);


/**
 * This property gives an output stream used to output log messages about
 * the execution of the algorithm.
 */
GenericIdentifier<elm::io::Output *>  ExeGraphBBTime::LOG_OUTPUT("otawa.ExeGraphBBTime.log", &cerr);


/**
 * Build the ExeGraphBBTime processor.
 * @param props	Configuration properties possibly including @ref PROC and
 * @ref LOG.
 */ 
ExeGraphBBTime::ExeGraphBBTime(const PropList& props) 
:	BBProcessor(props),
	microprocessor(PROCESSOR(props)),
	dumpFile(*LOG_OUTPUT(props))
{
	assert(microprocessor);
}




// ---------------------------------------------------
// buildPrologueList
// ---------------------------------------------------

void ExeGraphBBTime::buildPrologueList(
	BasicBlock * bb,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * prologue, 
	int capacity, 
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * prologue_list) {
	// prologues are recursively built by considering preceeding nodes
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		BasicBlock * pred = edge->source();
		elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prologue =
			new elm::genstruct::DLList<ExecutionGraphInstruction *> ;
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*prologue) ; inst ; inst++) {
			ExecutionGraphInstruction * eg_inst =
				new ExecutionGraphInstruction(inst->inst(), inst->basicBlock(), inst->codePart(), inst->index());
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
		    ExecutionGraphInstruction * eg_inst = 
				new ExecutionGraphInstruction(inst_list.last(), pred, PROLOGUE, index--);	
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



void ExeGraphBBTime::buildEpilogueList(
	BasicBlock * bb,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * epilogue, 
	int capacity, 
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * epilogue_list) {
	// epilogues are recursively built by considering succeeding nodes
//	LOG(dumpFile << "\tentering buildPrologueList with bb = b" << bb->number() << "\n";)
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		BasicBlock * succ = edge->target();
//		LOG(dumpFile << "\tpred = b" << pred->number() << "\n";)
		elm::genstruct::DLList<ExecutionGraphInstruction *> * new_epilogue =
			new elm::genstruct::DLList<ExecutionGraphInstruction *> ;
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*epilogue) ; inst ; inst++) {
			ExecutionGraphInstruction * eg_inst =
				new ExecutionGraphInstruction(inst->inst(), inst->basicBlock(), inst->codePart(), inst->index());
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
		   	index = bb->countInstructions();
		  while ( (new_epilogue->count() < capacity) && (!inst_list.isEmpty()) ) {
		    ExecutionGraphInstruction * eg_inst = 
				new ExecutionGraphInstruction(inst_list.first(), succ, EPILOGUE, index++);	
		    new_epilogue->addLast(eg_inst);
		    inst_list.removeFirst();
		  }
		  if (new_epilogue->count() < capacity)
		    buildEpilogueList(succ, new_epilogue, capacity, epilogue_list);
		  else {
		    epilogue_list->addLast(new_epilogue);
		    
		  }
		}
	}
	delete epilogue;
}



// ---------------------------------------------------
// processSequence
// ---------------------------------------------------

int ExeGraphBBTime::processSequence( FrameWork *fw,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * prologue,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * body,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * epilogue,
	int capacity ) {
		
	elm::genstruct::DLList<ExecutionGraphInstruction *> sequence;
	
	sequence.clear();
	if (prologue && (prologue->count() < capacity)) {
		int index = 0;
		if (!prologue->isEmpty())
			index = prologue->first()->index() - 1;
		ExecutionGraphInstruction * eg_inst = 
			new ExecutionGraphInstruction(NULL, NULL, BEFORE_PROLOGUE, index);
		sequence.addLast(eg_inst);
	}
	
	if (prologue) {
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*prologue) ; inst ; inst++) {
			sequence.addLast(inst);
		}
	}
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*body); inst ; inst++) {
		sequence.addLast(inst);
	}
	if (epilogue) {
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*epilogue) ; inst ; inst++) {
			sequence.addLast(inst);
		}
	}

	// set dump file
	elm::StringBuffer file_name;
	elm::String string_file_name, string_timed_file_name, extension, number, extension2;
	string_file_name = "./";
	string_timed_file_name = string_file_name;
	extension = ".dot";
	extension2 = "_times.dot";
	{
		code_part_t part = BEFORE_PROLOGUE;
		int bbnum = -1;
		file_name << body->first()->basicBlock()->number() << "+";
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ; inst ; inst++) {
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
	elm::io::OutFileStream dotStream(string_file_name.toCString());
	elm::io::Output dotFile(dotStream);	
			
	ExecutionGraph execution_graph;
	execution_graph.build(fw, microprocessor, sequence);
	LOG(execution_graph.dumpLight(dumpFile));
	int bbExecTime = execution_graph.analyze(dumpFile);
	
	LOG(dumpFile << "Cost of block " << body->first()->basicBlock()->number() << " is " << bbExecTime << "\n");
	string_timed_file_name = string_timed_file_name.concat(number);
	string_timed_file_name = string_timed_file_name.concat(extension2);
	elm::io::OutFileStream timedDotStream(string_timed_file_name.toCString());
	elm::io::Output timedDotFile(timedDotStream);
	// dump the execution graph *with times* in dot format
	execution_graph.dotDump(timedDotFile,true);		
	
	return bbExecTime;
}

// ---------------------------------------------------
// processBB
// ---------------------------------------------------

void ExeGraphBBTime::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	elm::genstruct::DLList<ExecutionGraphInstruction *> prologue, body, epilogue, sequence;
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> prologue_list, epilogue_list;
	
	LOG(dumpFile << "================================================================\n");
	LOG(dumpFile << "Processing block b" << bb->number() << ":\n\n");
	LOG(for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			dumpFile << inst->address() <<": ";
			inst->dump(dumpFile);
			dumpFile << "\n";	
		}
	)	
	
	if (bb->countInstructions() == 0)
		return;

	// compute prologue/epilogue size
	int capacity = 0;
	for(Microprocessor::QueueIterator queue(microprocessor); queue; queue++){
		capacity += queue->size();
	}
	LOG(dumpFile << "Processor capacity = " << capacity << "\n";) 

	// build the list of possible prologues
	elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prologue = 
		new elm::genstruct::DLList<ExecutionGraphInstruction *>;
	buildPrologueList(bb, new_prologue, capacity, &prologue_list);
	// dump prologue list
		LOG(	dumpFile << "Dumping the list of prologues:\n";
	    	int p =0;
	    	int index;
			if (!prologue_list.isEmpty()) {
				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
			  		assert(!prologue->isEmpty());
					dumpFile << "\tprologue " << p++ << ":\t";
					int bbnum = -1;
					int cnt = 0;
					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**prologue) ; inst ; inst++) {
						if (inst->basicBlock()->number() != bbnum) {
							if (bbnum != -1)
								dumpFile << index << ") - ";
							bbnum = inst->basicBlock()->number();
							dumpFile << "b" << bbnum ;
							dumpFile << " (i" << inst->index() << " to i";
							cnt = 0;
						}
						cnt++;
						index = inst->index();
					}
					if (!prologue->isEmpty())
						dumpFile << prologue->last()->index() << ") - ";
					dumpFile << "\n";
				}
			}
	)
	
		
	// build the list of body instructions	
	{
		int index = 1;
		for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			ExecutionGraphInstruction *eg_inst =
				new ExecutionGraphInstruction(inst, bb, BODY, index++);
			body.addLast(eg_inst);
		}
	}
	
	
		// build the list of possible epilogues
	elm::genstruct::DLList<ExecutionGraphInstruction *> * new_epilogue = 
		new elm::genstruct::DLList<ExecutionGraphInstruction *>;
		buildEpilogueList(bb, new_epilogue, capacity, &epilogue_list);
	// dump prologue list
		LOG(	dumpFile << "Dumping the list of epilogues:\n";
	    	{int p =0;
	    	int index;
			if (!epilogue_list.isEmpty()) {
				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
			  		assert(!epilogue->isEmpty());
					dumpFile << "\tepilogue " << p++ << ":\t";
					int bbnum = -1;
					int cnt = 0;
					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**epilogue) ; inst ; inst++) {
						if (inst->basicBlock()->number() != bbnum) {
							if (bbnum != -1)
								dumpFile << index << ") - ";
							bbnum = inst->basicBlock()->number();
							dumpFile << "b" << bbnum ;
							dumpFile << " (i" << inst->index() << " to i";
							cnt = 0;
						}
						cnt++;
						index = inst->index();
					}
					if (!epilogue->isEmpty())
						dumpFile << epilogue->last()->index() << ") - ";
					dumpFile << "\n";
				}
			}}
	)
	
	
	
	int maxExecTime = 0;
	int bbExecTime;
	
	// consider every possible prologue/epilogue pair
	if (prologue_list.isEmpty()){
		if (epilogue_list.isEmpty()) {
			LOG(dumpFile << "\nProcessing sequence: ";
				dumpFile << "[] [b" << bb->number() << "] []\n;";
			)				
			bbExecTime = processSequence(fw, NULL, &body, NULL, capacity);
			if (bbExecTime > maxExecTime)
				maxExecTime = bbExecTime;	
		}
		else {			
			for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
				LOG(dumpFile << "\nProcessing sequence: ";
					dumpFile << "[] [b" << bb->number() << "] [";				
					int bbnum = -1;
					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**epilogue) ; inst ; inst++) {
						if (inst->basicBlock()->number() != bbnum) {
							if (bbnum != -1)
								dumpFile << "-";
							bbnum = inst->basicBlock()->number();
							dumpFile << "b" << bbnum ;
						}
					}
					dumpFile << "]\n";
				)
				bbExecTime = processSequence(fw, NULL, &body, epilogue, capacity);
				if (bbExecTime > maxExecTime)
					maxExecTime = bbExecTime;	
			}
		}
	}
	else {
		if (epilogue_list.isEmpty()) {
			for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
				LOG(dumpFile << "\nProcessing sequence: ";
					dumpFile << "[";
					int bbnum = -1;
					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**prologue) ; inst ; inst++) {
						if (inst->basicBlock()->number() != bbnum) {
							if (bbnum != -1)
								dumpFile << "-";
							bbnum = inst->basicBlock()->number();
							dumpFile << "b" << bbnum ;
						}
					}
					dumpFile << "] [b" << bb->number() << "] []\n";				
				)
				bbExecTime = processSequence(fw, prologue, &body, NULL, capacity);bbExecTime = processSequence(fw, prologue, &body, NULL, capacity);
				if (bbExecTime > maxExecTime)
					maxExecTime = bbExecTime;	
			}
		}
		else {
			for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
					LOG(dumpFile << "\nProcessing sequence: ";
						dumpFile << "[";
						int bbnum = -1;
						for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**prologue) ; inst ; inst++) {
							if (inst->basicBlock()->number() != bbnum) {
								if (bbnum != -1)
									dumpFile << "-";
								bbnum = inst->basicBlock()->number();
								dumpFile << "b" << bbnum ;
							}
						}
						dumpFile << "] [b" << bb->number() << "] [";				
						bbnum = -1;
						for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**epilogue) ; inst ; inst++) {
							if (inst->basicBlock()->number() != bbnum) {
								if (bbnum != -1)
									dumpFile << "-";
								bbnum = inst->basicBlock()->number();
								dumpFile << "b" << bbnum ;
							}
						}
						dumpFile << "]\n";
					)
					bbExecTime = processSequence(fw, prologue, &body, epilogue, capacity);
					if (bbExecTime > maxExecTime)
						maxExecTime = bbExecTime;	
				}
			}
		}
		
	}
	
	// is the cost of the block for that prologue/epilogue pair the WCC of the block ?
	
	LOG(dumpFile << "\nWCC of block " << bb->number() << " is " << maxExecTime << "\n");
	bb->set<int>(TIME, maxExecTime);
}


}  // otawa
