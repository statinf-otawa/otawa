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
#include <otawa/hard/Processor.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>

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

//#define ACCURATE_STATS
#if defined(NDEBUG) || !defined(ACCURATE_STATS)
#	define STAT(c)
#else
#	define STAT(c) c
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
	dumpFile(*LOG_OUTPUT(props)),
	delta(EXEGRAPH_DELTA(props)),
	do_context(EXEGRAPH_CONTEXT(props)),
	stat_root(0, 0),
	exe_stats(*(new Vector<stat_t>())),
	fw(0)
{
}


/**
 */
void ExeGraphBBTime::processFrameWork(FrameWork *fw) {
	bool built = false, reset = false;
	this->fw = fw;
	
	// If required, find the microprocessor
	if(!microprocessor) {
		microprocessor = PROCESSOR(fw);
		if(microprocessor) 
			reset = true;
		else {
			const hard::Processor *proc = fw->platform()->processor();
			if(!proc)
				throw ProcessorException(*this, "no processor to work with");
			else {
				reset = true;
				built = true;
				microprocessor = new Microprocessor(proc);
			}
		}
	}
	
	// Perform the actual process
	BBProcessor::processFrameWork(fw);
	
	// Record stats if required
	if(recordsStats())
		EXEGRAPH_PREFIX_STATS(stats) = &exe_stats;

	// Cleanup if required
	if(built)
		delete microprocessor;
	if(reset)
		microprocessor = 0;
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
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * epilogue_list,
	int start_index) {
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
		   	index = start_index;
		  while ( (new_epilogue->count() < capacity) && (!inst_list.isEmpty()) ) {
		    ExecutionGraphInstruction * eg_inst = 
				new ExecutionGraphInstruction(inst_list.first(), succ, EPILOGUE, index++);	
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

// ---------------------------------------------------
// buildPrefixList
// ---------------------------------------------------

void ExeGraphBBTime::buildPrefixList(
	BasicBlock * bb,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * prefix, 
	int max_size, 
	int current_size,
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> * prefix_list) {
	// prefixes are recursively built by considering preceeding nodes
	if (max_size == 0)
		return;
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		BasicBlock * pred = edge->source();
		elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prefix =
			new elm::genstruct::DLList<ExecutionGraphInstruction *> ;
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*prefix) ; inst ; inst++) {
			ExecutionGraphInstruction * eg_inst =
				new ExecutionGraphInstruction(inst->inst(), inst->basicBlock(), inst->codePart(), inst->index());
			new_prefix->addLast(eg_inst);
		}
		if (pred->countInstructions() == 0) {
			if (!new_prefix->isEmpty()) {
				// current sequence is terminated (not more instructions to add)
				// (end of recursive building)	
				int index = 1;
				for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*new_prefix) ; inst ; inst++) {
					inst->setIndex(index++);
				}	 	 	
				prefix_list->addLast(new_prefix);
			}
		}	
		else {
		  // build new sequence from pred
		  elm::genstruct::DLList<Inst *> inst_list;
		  for(BasicBlock::InstIterator inst(pred); inst; inst++) {
		    inst_list.addLast(inst);
		  }
		  while ( !inst_list.isEmpty() ) {
		    ExecutionGraphInstruction * eg_inst = 
				new ExecutionGraphInstruction(inst_list.last(), pred, PREFIX, 0);	
		    new_prefix->addFirst(eg_inst);
		    inst_list.removeLast();
		  }
		  current_size++;
		  if (current_size < max_size)
		    buildPrefixList(pred, new_prefix, max_size, current_size, prefix_list);
		  else {
		  	int index = 1;
			for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*new_prefix) ; inst ; inst++) {
				inst->setIndex(index++);
			}	 	 	
	  		 prefix_list->addLast(new_prefix);
		    
		  }
		}
	}
	delete prefix;
}


// ---------------------------------------------------
// processSequence
// ---------------------------------------------------

int ExeGraphBBTime::processSequence( FrameWork *fw,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * prologue,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * prefix,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * body,
	elm::genstruct::DLList<ExecutionGraphInstruction *> * epilogue,
	int capacity ) {
		
	elm::genstruct::DLList<ExecutionGraphInstruction *> sequence;
	
	sequence.clear();
	if (prologue && (prologue->count() == capacity)) {
		int index = 0;
		index = prologue->first()->index() - 1;
		ExecutionGraphInstruction * eg_inst = 
			new ExecutionGraphInstruction(NULL, NULL, BEFORE_PROLOGUE, index);
		sequence.addLast(eg_inst);
	}
	
	LOG(dumpFile << "\n-------------------\nProcessing sequence : \n";) 
	
	LOG(dumpFile << "[prologue] ";)
	if (prologue) {
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*prologue) ; inst ; inst++) {
			sequence.addLast(inst);
			LOG(dumpFile << "i" << inst->index() << "(b" << inst->basicBlock()->number() << ") ";)
		}
	}	
	LOG(dumpFile << " [PREFIX] ";)	
	if (prefix) {
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*prefix) ; inst ; inst++) {
			sequence.addLast(inst);
			LOG(dumpFile << "i" << inst->index() << "(b" << inst->basicBlock()->number() << ") ";)
		}
	}	
	LOG(dumpFile << " [BODY] ";)	
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*body); inst ; inst++) {
		sequence.addLast(inst);
		LOG(dumpFile << "i" << inst->index() << "(b" << inst->basicBlock()->number() << ") ";)
	}
	LOG(dumpFile << " [EPILOGUE] ";)
	if (epilogue) {
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*epilogue) ; inst ; inst++) {
			sequence.addLast(inst);
			LOG(dumpFile << "i" << inst->index() << "(b" << inst->basicBlock()->number() << ") ";)
		}
	}
	LOG(dumpFile <<"\n";)

	// set dump file
	#ifdef DO_LOG
		elm::StringBuffer file_name;
		elm::String string_file_name, string_timed_file_name, extension, number, extension2;
		string_file_name = "./graphs/";
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
	#endif // DO_LOG	
			
	ExecutionGraph execution_graph(capacity);
	execution_graph.build(fw, microprocessor, sequence);
	LOG(execution_graph.dumpLight(dumpFile));
	int bbExecTime = execution_graph.analyze();
	if(this->recordsStats() || do_context)
		recordPrefixStats(prologue, bbExecTime);
	
	#ifdef DO_LOG
		dumpFile << "Cost of block " << body->first()->basicBlock()->number() << " is " << bbExecTime << "\n";
		string_timed_file_name = string_timed_file_name.concat(number);
		string_timed_file_name = string_timed_file_name.concat(extension2);
		elm::io::OutFileStream timedDotStream(string_timed_file_name.toCString());
		elm::io::Output timedDotFile(timedDotStream);
		// dump the execution graph *with times* in dot format
		LOG(execution_graph.dotDump(timedDotFile,true));		
	#endif // DO_LOG
	
	#ifdef ACCURATE_STATS
	int bbnum = -1;
	elm::genstruct::DLList<BasicBlock *> prologue_blocks;
	
	PrefixCost * prefix_cost, *existing_cost;
	bool found = false;
	for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost  && !found; existing_cost++) {
		found = existing_cost->isPrefix(&prologue_blocks);
		if (found)
			existing_cost->addCost(bbExecTime);
	}
	if (!found) {
		prefix_cost = new PrefixCost(&prologue_blocks);
		prefix_cost->addCost(bbExecTime);
		costs.addLast(prefix_cost);		
	}
	if (prefix) {
		prologue_blocks.addLast(prefix->first()->basicBlock()->number();
		bool found = false;
		for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost  && !found; existing_cost++) {
			found = existing_cost->isPrefix(&prologue_blocks);
			if (found)
				existing_cost->addCost(bbExecTime);
		}
		if (!found) {
			prefix_cost = new PrefixCost(&prologue_blocks);
			prefix_cost->addCost(bbExecTime);
			costs.addLast(prefix_cost);	
		}	
		for(DLList<ExecutionGraphInstruction *>::Iterator inst(prologue->fromLast()); inst; inst--) {
			if (inst->basicBlock()->number() != bbnum) {
				bbnum = inst->basicBlock()->number();
				prologue_blocks.addLast(inst->basicBlock());
				LOG(dumpFile << "prologue_blocks=";
					for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(prologue_blocks) ; bb ; bb++) {
						dumpFile << "b" << bb->number() << "-";
					}
					dumpFile << "\n";
				)
				bool found = false;
				for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost  && !found; existing_cost++) {
					found = existing_cost->isPrefix(&prologue_blocks);
					if (found)
						existing_cost->addCost(bbExecTime);
				}
				if (!found) {
					prefix_cost = new PrefixCost(&prologue_blocks);
					prefix_cost->addCost(bbExecTime);
					costs.addLast(prefix_cost);	
				}	
			}
		}
	}
	
#endif
	
	return bbExecTime;
}

// ---------------------------------------------------
// processBB
// ---------------------------------------------------

void ExeGraphBBTime::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	elm::genstruct::DLList<ExecutionGraphInstruction *> prologue, prefix, body, epilogue, sequence;
	
	costs.clear();

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
	
	// Start recording stats
	if(recordsStats() || do_context)
		initPrefixStats(bb);

	// compute prologue/epilogue size
	int capacity = 0;
	for(Microprocessor::QueueIterator queue(microprocessor); queue; queue++){
		capacity += queue->size();
		
	}
	LOG(dumpFile << "Processor capacity = " << capacity << "\n";) 
	

	#ifdef ACCURATE_STATS
		elm::StringBuffer file_name_buffer;
		file_name_buffer << "b" << bb->number() << ".stats";
		elm::String file_name;
		file_name = file_name_buffer.toString();
		elm::io::OutFileStream statsStream(file_name.toCString());
		elm::io::Output statsFile(statsStream);
	#endif	
	
	elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> prefix_list;
	elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prefix = 
			new elm::genstruct::DLList<ExecutionGraphInstruction *>;
	buildPrefixList(bb, new_prefix, 2, 0, &prefix_list);
	
	//dump prefix list
//		#ifdef DO_LOG
//			dumpFile << "Dumping the list of prefixes:\n";
//	    	int p =0;
//	    	int index;
//			if (!prefix_list.isEmpty()) {
//				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prefix(prefix_list) ; prefix ; prefix++) {
//			  		assert(!prefix->isEmpty());
//					dumpFile << "\tprefix " << p++ << ":\t";
//					int bbnum = -1;
//					int cnt = 0;
//					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**prefix) ; inst ; inst++) {
//						if (inst->basicBlock()->number() != bbnum) {
//							if (bbnum != -1)
//								dumpFile << index << ") - ";
//							bbnum = inst->basicBlock()->number();
//							dumpFile << "b" << bbnum ;
//							dumpFile << " (i" << inst->index() << " to i";
//							cnt = 0;
//						}
//						cnt++;
//						index = inst->index();
//					}
//					if (!prefix->isEmpty())
//						dumpFile << prefix->last()->index() << ") - ";
//					dumpFile << "\n";
//				}
//			}
//		#endif	
	
	
	int maxExecTime;
//	------------------------FIXME ------------------------------------
//	if(!do_context || prologue_list.isEmpty())
//		maxExecTime = 0;
//	else
//	maxExecTime = INFINITE_TIME;
	maxExecTime = 0;
	int bbExecTime;
			
		
//	int pred_count;
//	for(BasicBlock::InIterator edge(bb); edge; edge++) {
//		BasicBlock * pred = edge->source();
//		pred_count++;
	if (!prefix_list.isEmpty()) {
		for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prefix(prefix_list) ; prefix ; prefix++) {
		
			elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> prologue_list, epilogue_list;
			
			// build the list of possible prologues
			elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prologue = 
				new elm::genstruct::DLList<ExecutionGraphInstruction *>;
			buildPrologueList(prefix->first()->basicBlock(), new_prologue, capacity, &prologue_list);
	
			// dump prologue list
	//		#ifdef DO_LOG
	//			dumpFile << "Dumping the list of prologues:\n";
	//	    	int p =0;
	//	    	int index;
	//			if (!prologue_list.isEmpty()) {
	//				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
	//			  		assert(!prologue->isEmpty());
	//					dumpFile << "\tprologue " << p++ << ":\t";
	//					int bbnum = -1;
	//					int cnt = 0;
	//					for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**prologue) ; inst ; inst++) {
	//						if (inst->basicBlock()->number() != bbnum) {
	//							if (bbnum != -1)
	//								dumpFile << index << ") - ";
	//							bbnum = inst->basicBlock()->number();
	//							dumpFile << "b" << bbnum ;
	//							dumpFile << " (i" << inst->index() << " to i";
	//							cnt = 0;
	//						}
	//						cnt++;
	//						index = inst->index();
	//					}
	//					if (!prologue->isEmpty())
	//						dumpFile << prologue->last()->index() << ") - ";
	//					dumpFile << "\n";
	//				}
	//			}
	//		#endif	
			
			
			{
				// build the list of prefix instructions	
//				prefix.clear();
//				int index = 1;
//				for(BasicBlock::InstIterator inst(pred); inst; inst++) {
//					ExecutionGraphInstruction *eg_inst =
//						new ExecutionGraphInstruction(inst, pred, PREFIX, index++);
//					prefix.addLast(eg_inst);
//				}
				int index = prefix->count() + 1;
				// build the list of body instructions	
				body.clear();
				for(BasicBlock::InstIterator inst(bb); inst; inst++) {
					ExecutionGraphInstruction *eg_inst =
						new ExecutionGraphInstruction(inst, bb, BODY, index++);
					body.addLast(eg_inst);
				}
			}
				
			// build the list of possible epilogues
			elm::genstruct::DLList<ExecutionGraphInstruction *> * new_epilogue = 
				new elm::genstruct::DLList<ExecutionGraphInstruction *>;
			buildEpilogueList(bb, new_epilogue, capacity, &epilogue_list, 
							prefix->count() + body.count() + 1);
		
			// dump epilogue list
	//			LOG(	dumpFile << "Dumping the list of epilogues:\n";
	//		    	{int p =0;
	//		    	int index;
	//				if (!epilogue_list.isEmpty()) {
	//					for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
	//				  		assert(!epilogue->isEmpty());
	//						dumpFile << "\tepilogue " << p++ << ":\t";
	//						int bbnum = -1;
	//						int cnt = 0;
	//						for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(**epilogue) ; inst ; inst++) {
	//							if (inst->basicBlock()->number() != bbnum) {
	//								if (bbnum != -1)
	//									dumpFile << index << ") - ";
	//								bbnum = inst->basicBlock()->number();
	//								dumpFile << "b" << bbnum ;
	//								dumpFile << " (i" << inst->index() << " to i";
	//								cnt = 0;
	//							}
	//							cnt++;
	//							index = inst->index();
	//						}
	//						if (!epilogue->isEmpty())
	//							dumpFile << epilogue->last()->index() << ") - ";
	//						dumpFile << "\n";
	//					}
	//				}}
	//		)
				
						
			// consider every possible prologue/epilogue pair
			if (prologue_list.isEmpty()){
				if (epilogue_list.isEmpty()) {
					bbExecTime = processSequence(fw, NULL, prefix, &body, NULL, capacity);
					
	//				#ifdef ACCURATE_STATS
	//					PrefixCost * prefix_cost;
	//					elm::genstruct::DLList<BasicBlock *> prologue_blocks;
	//					prologue_blocks.addFirst(pred);
	//					bool found = false;
	//					for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost && !found ; existing_cost++) {
	//						found = existing_cost->isPrefix(&prologue_blocks);
	//						if (found)
	//							existing_cost->addCost(bbExecTime);
	//					}
	//					if (!found) {
	//						prefix_cost = new PrefixCost(&prologue_blocks);
	//						prefix_cost->addCost(bbExecTime);
	//						costs.addLast(prefix_cost);	
	//					}
	//				#endif
					if (bbExecTime > maxExecTime)
						maxExecTime = bbExecTime;
				}
				else {			
					for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
						bbExecTime = processSequence(fw, NULL, prefix, &body, epilogue, capacity);
						
	//					#ifdef ACCURATE_STATS
	//						PrefixCost * prefix_cost;
	//						elm::genstruct::DLList<BasicBlock *> prologue_blocks;
	//						
	//						bool found = false;
	//						for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost && !found; existing_cost++) {
	//							found = existing_cost->isPrefix(&prologue_blocks);
	//							if (found)
	//								existing_cost->addCost(bbExecTime);
	//						}
	//						if (!found) {
	//							prefix_cost = new PrefixCost(&prologue_blocks);
	//							prefix_cost->addCost(bbExecTime);
	//							costs.addLast(prefix_cost);		
	//						}
	//					#endif
						
						if (bbExecTime > maxExecTime)
							maxExecTime = bbExecTime;
					}
				}
			}
			else {
				if (epilogue_list.isEmpty()) {
					for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
						bbExecTime = processSequence(fw, prologue, prefix, &body, NULL, capacity);
						if(delta)
							recordDelta(prologue, bbExecTime, bb);
						
	//					#ifdef ACCURATE_STATS
	//						bbnum = -1;
	//						elm::genstruct::DLList<BasicBlock *> prologue_blocks;
	//						
	//						PrefixCost * prefix_cost, *existing_cost;
	//						bool found = false;
	//						for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost && !found; existing_cost++) {
	//							found = existing_cost->isPrefix(&prologue_blocks);
	//							if (found)
	//								existing_cost->addCost(bbExecTime);
	//						}
	//						if (!found) {
	//							prefix_cost = new PrefixCost(&prologue_blocks);
	//							prefix_cost->addCost(bbExecTime);
	//							costs.addLast(prefix_cost);	
	//						}
	//						for(DLList<ExecutionGraphInstruction *>::Iterator
	//						inst((*prologue)->fromLast()); inst; inst--) {
	//						//for (inst.last() ; !inst.begining() ; inst.previous()) {
	//							if (inst->basicBlock()->number() != bbnum) {					
	//								bbnum = inst->basicBlock()->number();
	//								prologue_blocks.addLast(inst->basicBlock());
	//								bool found = false;
	//								for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost && !found; existing_cost++) {
	//									found = existing_cost->isPrefix(&prologue_blocks);
	//									if (found)
	//										existing_cost->addCost(bbExecTime);
	//								}
	//								if (!found) {							
	//									prefix_cost = new PrefixCost(&prologue_blocks);
	//									prefix_cost->addCost(bbExecTime);
	//									costs.addLast(prefix_cost);	
	//								}	
	//							}
	//						}
	//					#endif
						
						if(!do_context) {
							if (bbExecTime > maxExecTime)
								maxExecTime = bbExecTime;
						}
						else {
							if (bbExecTime < maxExecTime)
								maxExecTime = bbExecTime;
						}	
					}
				}
				else {
					for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
						for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
							bbExecTime = processSequence(fw, prologue, prefix, &body, epilogue, capacity);
							if(delta)
								recordDelta(prologue, bbExecTime, bb);
						
	//						#ifdef ACCURATE_STATS
	//							bbnum = -1;
	//							elm::genstruct::DLList<BasicBlock *> prologue_blocks;
	//							
	//							PrefixCost * prefix_cost, *existing_cost;
	//							bool found = false;
	//							for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost  && !found; existing_cost++) {
	//								found = existing_cost->isPrefix(&prologue_blocks);
	//								if (found)
	//									existing_cost->addCost(bbExecTime);
	//							}
	//							if (!found) {
	//								prefix_cost = new PrefixCost(&prologue_blocks);
	//								prefix_cost->addCost(bbExecTime);
	//								costs.addLast(prefix_cost);		
	//							}
	//							for(DLList<ExecutionGraphInstruction *>::Iterator
	//							inst(prologue->fromLast()); inst; inst--) {
	//								if (inst->basicBlock()->number() != bbnum) {
	//									bbnum = inst->basicBlock()->number();
	//									prologue_blocks.addLast(inst->basicBlock());
	//									LOG(dumpFile << "prologue_blocks=";
	//										for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(prologue_blocks) ; bb ; bb++) {
	//											dumpFile << "b" << bb->number() << "-";
	//										}
	//										dumpFile << "\n";
	//									)
	//									bool found = false;
	//									for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost  && !found; existing_cost++) {
	//										found = existing_cost->isPrefix(&prologue_blocks);
	//										if (found)
	//											existing_cost->addCost(bbExecTime);
	//									}
	//									if (!found) {
	//										prefix_cost = new PrefixCost(&prologue_blocks);
	//										prefix_cost->addCost(bbExecTime);
	//										costs.addLast(prefix_cost);	
	//									}	
	//								}
	//							}
	//						#endif
							
							LOG(dumpFile << "\n";)		
							if(!do_context) {
								if (bbExecTime > maxExecTime)
									maxExecTime = bbExecTime;
							}	
							else {
								if (bbExecTime < maxExecTime)
									maxExecTime = bbExecTime;
							}	
						}
					}
				}
				
			}
		}
	}
	else { // no prefixes
//	if (pred_count == 0) {  // block has no predecessor
		elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *> prologue_list, epilogue_list;
		// build the list of body instructions	
		int index = 1;
		for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			ExecutionGraphInstruction *eg_inst =
				new ExecutionGraphInstruction(inst, bb, BODY, index++);
			body.addLast(eg_inst);
		}
			
		// build the list of possible prologues
		elm::genstruct::DLList<ExecutionGraphInstruction *> * new_prologue = 
			new elm::genstruct::DLList<ExecutionGraphInstruction *>;
		buildPrologueList(bb, new_prologue, capacity, &prologue_list);
			
		// build the list of possible epilogues
		elm::genstruct::DLList<ExecutionGraphInstruction *> * new_epilogue = 
			new elm::genstruct::DLList<ExecutionGraphInstruction *>;
		buildEpilogueList(bb, new_epilogue, capacity, &epilogue_list, body.count() + 1);
					
		if (prologue_list.isEmpty()) {
			if (epilogue_list.isEmpty()) {
				bbExecTime = processSequence(fw, NULL, NULL, &body, NULL, capacity);
				if (bbExecTime > maxExecTime)
					maxExecTime = bbExecTime;
			}
			else {
				for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
					bbExecTime = processSequence(fw, NULL, NULL, &body, epilogue, capacity);
					if (bbExecTime > maxExecTime)
						maxExecTime = bbExecTime;	
				}
			}
		}
		else {
			for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator prologue(prologue_list) ; prologue ; prologue++) {
				if (epilogue_list.isEmpty()) {
					bbExecTime = processSequence(fw, prologue, NULL, &body, NULL, capacity);
					if (bbExecTime > maxExecTime)
						maxExecTime = bbExecTime;	
				}
				else {
					for (elm::genstruct::DLList<elm::genstruct::DLList<ExecutionGraphInstruction *> *>::Iterator epilogue(epilogue_list) ; epilogue ; epilogue++) {
						bbExecTime = processSequence(fw, prologue, NULL, &body, epilogue, capacity);
						if (bbExecTime > maxExecTime)
							maxExecTime = bbExecTime;		
					}
				}	
			}		
		}
					
//			#ifdef ACCURATE_STATS
//				PrefixCost * prefix_cost;
//				elm::genstruct::DLList<BasicBlock *> prologue_blocks;
//				prologue_blocks.addFirst(pred);
//				bool found = false;
//				for (elm::genstruct::DLList<PrefixCost *>::Iterator existing_cost(costs) ; existing_cost && !found ; existing_cost++) {
//					found = existing_cost->isPrefix(&prologue_blocks);
//					if (found)
//						existing_cost->addCost(bbExecTime);
//				}
//				if (!found) {
//					prefix_cost = new PrefixCost(&prologue_blocks);
//					prefix_cost->addCost(bbExecTime);
//					costs.addLast(prefix_cost);	
//				}
//			#endif
	}
		
	
	#ifdef ACCURATE_STATS
		for (elm::genstruct::DLList<PrefixCost *>::Iterator cost(costs) ; cost ; cost++) {
			statsFile << "Costs for prefix ";
			if (cost->reversePrefix()->isEmpty())
				statsFile << "NULL";
			else
				for (elm::genstruct::DLList<BasicBlock *>::Iterator bb(*(cost->reversePrefix())) ; bb ; bb++) {
					statsFile << "b" << bb->number() << "-";
				}
			statsFile << "\n\t";
			for (PrefixCost::CostIterator c(cost) ; c ; c++) {
				statsFile << (int) c << ", ";
			}
			statsFile << "\n\n";
		}
		costs.clear();
		// is the cost of the block for that prologue/epilogue pair the WCC of the block ?
	#endif
	
	LOG(	dumpFile << "WCC of block " << bb->number() << " is " << maxExecTime << "\n";
		)
	bb->set<int>(TIME, maxExecTime);
	
	// Stop recording stats
	if(do_context)
		buildContext(maxExecTime);
	if(this->recordsStats() || do_context)
		collectPrefixStats();
	
	// Fix the delta
	for(BasicBlock::InIterator edge(bb); edge; edge++)
		TIME_DELTA(edge) = TIME_DELTA(edge) - maxExecTime;			
}


/**
 * Initialize statistics collection for the given basic block.
 * @param bb	Basic block to use.
 */
void ExeGraphBBTime::initPrefixStats(BasicBlock *bb) {
	stat_root.bb = bb;
	stat_root.children = 0;
	stat_root.sibling = 0;
	stat_root.max = 0;
	stat_root.min = INFINITE_TIME;
	stat_root.vals.setLength(0);
}


/**
 * Record statistics for the given sequence.
 * @param sequence	Sequence to use.
 * @param cost		Cost of the sequence.
 */
void ExeGraphBBTime::recordPrefixStats(
	DLList<ExecutionGraphInstruction *> *insts,
	int cost)
{
	assert(cost >= 0);
	
	// Initialization
	int depth = 0;
	node_stat_t *node = &stat_root;
	recordPrefixNodeStats(node, cost);
	
	// Traverse backward the prolog to get the prefix
	if(insts)
		for(DLList<ExecutionGraphInstruction *>::Iterator inst(insts->fromLast());
		inst; inst--)
			if(inst->basicBlock() != node->bb)
				node = recordPrefixNode(node, inst->basicBlock(), cost); 
}


/**
 * Record the given node in the prefix tree.
 * @param parent	Parent node.
 * @param bb		Current BB.
 * @param cost		Cost of the current prefix.
 * @return			Node matching the given BB.
 */
ExeGraphBBTime::node_stat_t *ExeGraphBBTime::recordPrefixNode(
	node_stat_t *parent,
	BasicBlock *bb,
	int cost)
{
	assert(parent);
	assert(bb);
	assert(cost >= 0);
	node_stat_t *node = 0;
	
	// Look for the matching node
	for(node_stat_t *cur = parent->children; cur; cur = cur->sibling)
		if(cur->bb == bb) {
			node = cur;
			break;
		}
	
	// If not found, create it
	if(!node) {
		node = new node_stat_t(bb, cost);
		node->sibling = parent->children;
		parent->children = node;
	}
	
	// Record the stats
	recordPrefixNodeStats(node, cost);
	return node;
}

/**
 * Record statistics for the given node.
 * @param node	Node to record stats for.
 * @param cost	Cost of the current prefix.
 */
void ExeGraphBBTime::recordPrefixNodeStats(node_stat_t *node, int cost) {
	if(cost < node->min)
		node->min = cost;
	if(cost > node->max)
		node->max = cost;
	bool found = false;
	for(int i = 0; i < node->vals.length(); i++)
		if(cost == node->vals[i]) {
			found = true;
			break;
		}
	if(!found)
		node->vals.add(cost);
}




/**
 * Call to end the statistics of the current basic block.
 */
void ExeGraphBBTime::collectPrefixStats(int depth, node_stat_t *node) {
	assert(depth >= 0);

	// If required, initialize to root node
	if(!node)
		node = &stat_root;
	
	// Record local information
	while(depth >= exe_stats.length())
		exe_stats.add();
	exe_stats[depth].seq_cnt++;
	exe_stats[depth].bb_span_sum += node->max - node->min;
	exe_stats[depth].bb_vals_sum += node->vals.length();
	for(node_stat_t *child = node->children, *sibling; child; child = sibling) {
		sibling = child->sibling;
		collectPrefixStats(depth + 1, child);
		delete child;
	}
	
	// Totalize
	if(depth == 0)
		for(int i = 0; i < exe_stats.length(); i++)
			if(exe_stats[i].seq_cnt) {
				exe_stats[i].bb_cnt++;
				exe_stats[i].total_span_sum +=
					(double)exe_stats[i].bb_span_sum / exe_stats[i].seq_cnt;
				exe_stats[i].total_vals_sum +=
					(double)exe_stats[i].bb_vals_sum / exe_stats[i].seq_cnt;
				exe_stats[i].seq_cnt = 0;
				exe_stats[i].bb_span_sum = 0;
				exe_stats[i].bb_vals_sum = 0;
			}
}


/**
 * Record the time delta on the input edges of the current block.
 * @param insts	Prologue.
 * @param cost	Cost of the block with the current prologue.
 * @param bb	Current basic block.
 */
void ExeGraphBBTime::recordDelta(
	DLList<ExecutionGraphInstruction *> *insts,
	int cost,
	BasicBlock *bb)
{
	assert(insts);
	assert(!insts->isEmpty());
	assert(bb);
	Inst *inst = insts->last()->inst();
	bool found = false;
	for(BasicBlock::InIterator edge(bb); edge; edge++) {
		BasicBlock *target = edge->target();
		if(target
		&& target->address() <= inst->address()
		&& inst->address() <= target->address() + target->size()
		&& TIME_DELTA(target) < cost) {
			TIME_DELTA(target) = cost;
			found = true;
		}
	}
	assert(found);
}


/**
 * If the statistics are activated, this property is returned in statistics
 * property list to store @ref ExeGraphBBTime statistics.
 */
GenericIdentifier<Vector <ExeGraphBBTime::stat_t> *>
	EXEGRAPH_PREFIX_STATS("exegraph_prefix_stats", 0, OTAWA_NS);


/**
 * If set to the true, ExeGraph will also put @ref TIME_DELTA properties on
 * edges. Using the @ref TimeDetaObjectFunctionModifier, it allow to improve
 * the accuracy of the computed WCET.
 */
GenericIdentifier<bool> EXEGRAPH_DELTA("exegraph_delta", false, OTAWA_NS);


/**
 * Set to true in the configuration @ref ExeGraphBBTime configure, this
 * processor will generate extra contraints and objects functions factor
 * taking in account the difference of execution time according prologues
 * of evaluated blocks.
 */
GenericIdentifier<bool> EXEGRAPH_CONTEXT("exegraph_context", false, OTAWA_NS);


/**
 * Compute extra context to handle context.
 * @param cost	Cost of the BB.
 * @param stat	Statistics node.
 * @param ctx	Context previous node.
 */
void ExeGraphBBTime::buildContext(
	int cost,
	node_stat_t *stat,
	context_t *ctx)
{
	// Build current context
	if(!stat) {
		stat = &stat_root;
		cost = stat->min;
	}
	context_t cctx(stat->bb, ctx);
	
	// Go into children
	if(stat->children)
		for(node_stat_t *cur = stat->children; cur; cur = cur->sibling)
			buildContext(cost, cur, &cctx);
	
	// Need to add extra constraint or object function factor ?
	else if(stat->min > cost) {
		
		// Add object function extra
		int delta = stat->min - cost;
		ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
		ilp::Var *var = system->newVar();
		system->addObjectFunction(delta, var);
		
		// Add edge constraints 
		for(context_t *cur = ctx, *prev = &cctx; cur; prev = cur, cur = cur->prev) {
		 	
		 	// find edge
		 	Edge *edge = 0;
		 	for(BasicBlock::OutIterator iter(prev->bb); iter; iter++)
		 		if(iter->target() == cur->bb) {
		 			edge = iter;
		 			break;
		 		}
		 	assert(edge);
		 	
		 	// add constraint
		 	ilp::Constraint *cons = system->newConstraint(ilp::Constraint::LE);
		 	cons->addLeft(1, var);
		 	cons->addRight(1, getVar(system, edge));
		 }
	}
} 

}  // otawa
