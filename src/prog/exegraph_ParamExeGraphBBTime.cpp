/*
 *	$Id$
 *	exegraph module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/io/OutFileStream.h>
#include <otawa/exegraph/ParamExeGraphBBTime.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;
using namespace otawa::ipet;

//#define ACCURATE_STATS
#if defined(NDEBUG) || !defined(ACCURATE_STATS)
#	define STAT(c)
#else
#	define STAT(c) c
#endif

namespace otawa { 

Identifier<String> OUTPUT_FILE("output file name", "");
Identifier<int> OUTPUT_LEVEL("output level", 0);
Identifier<String> GRAPHS_DIR("graphs directory","");



ParamExeGraphBBTime::ParamExeGraphBBTime(const PropList& props, int prologue_depth) 
: ExeGraphBBTime<ParamExeGraph>(props), _prologue_depth(prologue_depth) {
  if (!OUTPUT_FILE(props).isEmpty()) {
    String file_name = OUTPUT_FILE(props);
    _output_stream = new io::OutFileStream(file_name);
  }
  else
    _output_stream = &(elm::cout.stream());
  _output = new Output(*_output_stream);
  int level = OUTPUT_LEVEL(props);
  _trace = new Trace(*_output_stream, level,  "ExeGraph");
  
  _graphs_dir_name = GRAPHS_DIR(props);
}

void ParamExeGraphBBTime::buildPrologueList(BasicBlock * bb,
					    ExeSequence<ExeNode> * prologue, 
					    int capacity, 
					    elm::genstruct::DLList<ExeSequence<ExeNode> *> * prologue_list,
					    int depth) {
  // prologues are recursively built by considering preceeding nodes in the CFG
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
      while ( !inst_list.isEmpty() ) {
	ExeInst<ExeNode> * eg_inst = 
	  new ExeInst<ExeNode>(inst_list.last(), pred, ParamExeGraph::PROLOGUE, 0);	
	new_prologue->addFirst(eg_inst);
	inst_list.removeLast();
      }
      if ((new_prologue->count() < capacity)  // capacity = width of the last pipeline stage
            ||
	  (depth < _prologue_depth) ) 
	buildPrologueList(pred, new_prologue, capacity, prologue_list,depth+1);
      else {
	prologue_list->addLast(new_prologue);   
      }
    }
  }
  delete prologue;
  
}
  

/**
 */
void ParamExeGraphBBTime::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
  if (bb->countInstructions() == 0)
    return;
  
  *_output << "================================================================\n";
  *_output << "Processing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n\n";

  int context_index = 0;
  int maxExecTime = 0;
  elm::genstruct::DLList<ExeSequence<ExeNode> *> prologue_list;
   
  int capacity;
  for (ParamExeGraph::PipelineIterator stage(microprocessor) ; stage ; stage++) {
    capacity = stage->width();
  }
  
  ExeSequence<ParamExeNode> * new_prologue = new ExeSequence<ParamExeNode>();
  buildPrologueList(bb, new_prologue, capacity, &prologue_list,0);
  
  if (prologue_list.isEmpty()) {
    ExeSequence<ExeNode> sequence;
    int index = 0;
    for(BasicBlock::InstIterator inst(bb); inst; inst++) {
      ExeInst<ExeNode> *eg_inst = new ExeInst<ExeNode>(inst, bb, ParamExeGraph::BODY, index++);
      sequence.addLast(eg_inst);
    }
    bool has_pred = false;
    ParamExeGraph execution_graph(ws, microprocessor, &sequence, has_pred, _trace);
    execution_graph.build(fw, microprocessor, &sequence);
    int bbExecTime = execution_graph.analyze();
    *_output << "Context " << context_index << " (sequence b" << bb->number() << "): wcc = " << bbExecTime << "\n";
    maxExecTime = bbExecTime;
    // dump the execution graph *with times* in dot format
    if (!_graphs_dir_name.isEmpty()){
      elm::StringBuffer buffer;
      buffer << _graphs_dir_name << "/";
      buffer << bb->number() << "-c" << context_index << ".dot";
      elm::String string = buffer.toString();
      elm::io::OutFileStream dotStream(string);
      elm::io::Output dotFile(dotStream);
      execution_graph.dump(dotFile);
    }		
  }
  else {
    for (elm::genstruct::DLList<ExeSequence<ExeNode> *>::Iterator sequence(prologue_list) ; sequence ; sequence++) {
      int index = 0;
      for (ExeSequence<ExeNode>::InstIterator inst(sequence) ; inst ; inst++) {
				inst->setIndex(index++);
      }
      for(BasicBlock::InstIterator inst(bb); inst; inst++) {
				ExeInst<ExeNode> *eg_inst =
	  		new ExeInst<ExeNode>(inst, bb, ParamExeGraph::BODY, index++);
				sequence->addLast(eg_inst);
      }
      BasicBlock * first_bb = sequence->first()->basicBlock();
      bool has_pred = false;
      for(BasicBlock::InIterator edge(first_bb); edge & !has_pred; edge++) {
				BasicBlock * pred = edge->source();
				if (pred->countInstructions() != 0)
	  			has_pred = true;
      }
      
      int bbnum = -1;
      address_t bbadd = 0;
      
      ParamExeGraph execution_graph(fw, microprocessor, sequence, has_pred, _trace);
      execution_graph.build(fw, microprocessor, sequence);
    	
      int bbExecTime = execution_graph.analyze();
      *_output << "Context " << context_index << " (sequence ";
      for (ExeSequence<ExeNode>::InstIterator inst(sequence) ; inst ; inst++) {
	if ((inst->basicBlock()->number() != bbnum) ||(inst->inst()->address() == bbadd)) {
	  bbnum = inst->basicBlock()->number();
	  bbadd = inst->inst()->address();
	  *_output << "b" << bbnum <<  "-";
	}
      }

      *_output << "): wcc = " << bbExecTime << "\n";


      if (bbExecTime > maxExecTime)
	maxExecTime = bbExecTime;
      if (!_graphs_dir_name.isEmpty()){
	elm::StringBuffer buffer;
	buffer << _graphs_dir_name << "/";
	buffer << bb->number() << "-c" << context_index << ".dot";
	elm::String string = buffer.toString();
	elm::io::OutFileStream dotStream(string);
	elm::io::Output dotFile(dotStream);
	execution_graph.dump(dotFile);
      }	
      context_index++;
	
    }
  }
  *_output << "WCC of block " << bb->number() << " is " << maxExecTime << "\n";
  TIME(bb) = maxExecTime;
}
  
/**
 * @class ExecutionGraphBBTime
 * This basic block processor computes the basic block execution time using
 * the execution graph method described in the following papers:
 * @li X. Param, A. Roychoudhury, A., T. Mitra, "Modeling Out-of-Order Processors
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


//template ExecutionGraphBBTime<ParamExeGraph>;

}  // otawa
